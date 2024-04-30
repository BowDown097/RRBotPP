#include "administration.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/utils/strings.h"
#include "utils/ld.h"
#include "utils/rrutils.h"
#include <dpp/cluster.h>
#include <mongocxx/collection.hpp>

Administration::Administration() : dpp::module_base("Administration", "Commands for admin stuff. Whether you wanna screw with the economy or fuck someone over, I'm sure you'll have fun. However, you'll need to have a very high role to have all this fun. Sorry!")
{
    register_command(&Administration::clearTextChannel, "cleartextchannel", "Deletes and recreates a text channel, effectively wiping its messages.", "$cleartextchannel [channel]");
    register_command(&Administration::drawPot, "drawpot", "Draw the pot before it ends.");
    register_command(&Administration::removeAchievement, std::initializer_list<std::string> { "removeachievement", "rmach" }, "Remove a user's achievement.", "$removeachievement [user] [name]");
    register_command(&Administration::removeCrates, std::initializer_list<std::string> { "removecrates", "rmcrates" }, "Remove a user's crates.", "$removecrates [user]");
    register_command(&Administration::removeStat, std::initializer_list<std::string> { "removestat", "rmstat" }, "Remove a user's stat.", "$removestat [user] [stat]");
    register_command(&Administration::resetCooldowns, "resetcd", "Reset a user's cooldowns.", "$resetcd [user]");
    register_command(&Administration::setCash, "setcash", "Set a user's cash.", "$setcash [user] [amount]");
    register_command(&Administration::setCrypto, "setcrypto", "Set a user's cryptocurrency amount. See $invest's help info for currently accepted currencies.", "$setcrypto [user] [crypto] [amount]");
    register_command(&Administration::setPrestige, "setprestige", "Set a user's prestige level.", "$setprestige [user] [level]");
    register_command(&Administration::setStat, "setstat", "Set a stat for a user.", "$setstat [user] [stat] [value]");
    register_command(&Administration::unlockAchievement, "unlockachievement", "Unlock an achievement for a user.", "$unlockachievement [user] [achievement]");
}

dpp::task<dpp::command_result> Administration::clearTextChannel(const dpp::channel_in& channelIn)
{
    dpp::channel* channel = channelIn.top_result();
    co_await cluster->co_channel_delete(channel->id);
    co_await cluster->co_channel_create(*channel);
    co_return dpp::command_result::from_success();
}

dpp::command_result Administration::drawPot()
{
    DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
    if (pot.endTime < RR::utility::unixTimeSecs())
        return dpp::command_result::from_error(Responses::PotEmpty);

    pot.endTime = 69;
    MongoManager::updatePot(pot);
    return dpp::command_result::from_success(Responses::PotDrawing);
}

dpp::command_result Administration::removeAchievement(const dpp::user_in& userIn, const dpp::remainder<std::string>& name)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (!std::erase_if(dbUser.achievements, [&name](auto& p) { return dpp::utility::iequals(p.first, *name); }))
        return dpp::command_result::from_error(std::format(Responses::MissingAchievement, user->get_mention()));

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedAchievement, user->get_mention()));
}

dpp::command_result Administration::removeCrates(const dpp::user_in& userIn)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.crates.clear();

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedCrates, user->get_mention()));
}

dpp::command_result Administration::removeStat(const dpp::user_in& userIn, const dpp::remainder<std::string>& stat)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (!std::erase_if(dbUser.stats, [&stat](auto& p) { return dpp::utility::iequals(p.first, *stat); }))
        return dpp::command_result::from_error(std::format(Responses::MissingStat, user->get_mention()));

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedStat, user->get_mention()));
}

dpp::command_result Administration::resetCooldowns(const dpp::user_in& userIn)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.bullyCooldown = dbUser.chopCooldown = dbUser.cocaineRecoveryTime = dbUser.dailyCooldown =
    dbUser.dealCooldown = dbUser.digCooldown = dbUser.farmCooldown = dbUser.fishCooldown =
    dbUser.hackCooldown = dbUser.huntCooldown = dbUser.lootCooldown = dbUser.mineCooldown =
    dbUser.pacifistCooldown = dbUser.prestigeCooldown = dbUser.rapeCooldown = dbUser.robCooldown =
    dbUser.scavengeCooldown = dbUser.shootCooldown = dbUser.slaveryCooldown = dbUser.whoreCooldown = 0;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::ResetCooldowns, user->get_mention()));
}

dpp::task<dpp::command_result> Administration::setCash(const dpp::guild_member_in& memberIn, const cash_in& amountIn)
{
    long double amount = amountIn.top_result();
    if (amount < 0)
        co_return dpp::command_result::from_error(Responses::NegativeCash);

    dpp::guild_member member = memberIn.top_result();
    dpp::user* user = member.get_user();
    if (!user)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    co_await dbUser.setCashWithoutAdjustment(member, amount, cluster, context);

    MongoManager::updateUser(dbUser);
    co_return dpp::command_result::from_success(std::format(Responses::SetCash,
        user->get_mention(), RR::utility::currencyToStr(amount)));
}

dpp::command_result Administration::setCrypto(const dpp::user_in& userIn, const std::string& crypto, long double amount)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    std::string cryptoLower;
    std::ranges::transform(crypto, std::back_inserter(cryptoLower), tolower);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (cryptoLower == "btc")
        dbUser.btc = amount;
    else if (cryptoLower == "eth")
        dbUser.eth = amount;
    else if (cryptoLower == "ltc")
        dbUser.ltc = amount;
    else if (cryptoLower == "xrp")
        dbUser.xrp = amount;
    else
        return dpp::command_result::from_error(Responses::InvalidCrypto);

    std::string cryptoUpper;
    std::ranges::transform(crypto, std::back_inserter(cryptoUpper), toupper);

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetCrypto, user->get_mention(), cryptoUpper, amount));
}

dpp::command_result Administration::setPrestige(const dpp::user_in& userIn, int level)
{
    if (level < 0 || level > Constants::MaxPrestige)
        return dpp::command_result::from_error(Responses::InvalidPrestigeLevel);

    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.prestige = level;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetPrestige, user->get_mention(), level));
}

dpp::command_result Administration::setStat(const dpp::user_in& userIn, const std::string& stat,
                                            const dpp::remainder<std::string>& value)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.stats[stat] = *value;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetStat, user->get_mention(), stat, *value));
}

dpp::command_result Administration::unlockAchievement(const dpp::user_in& userIn, const dpp::remainder<std::string>& name)
{
    dpp::user* user = userIn.top_result();
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.unlockAchievement(*name, context);

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success();
}
