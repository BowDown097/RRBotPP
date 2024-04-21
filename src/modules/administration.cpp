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

Administration::Administration() : ModuleBase("Administration", "Commands for admin stuff. Whether you wanna screw with the economy or fuck someone over, I'm sure you'll have fun. However, you'll need to have a very high role to have all this fun. Sorry!")
{
    registerCommand(&Administration::clearTextChannel, "cleartextchannel", "Deletes and recreates a text channel, effectively wiping its messages.", "$cleartextchannel [channel]");
    registerCommand(&Administration::drawPot, "drawpot", "Draw the pot before it ends.");
    registerCommand(&Administration::removeAchievement, std::initializer_list<std::string> { "removeachievement", "rmach" }, "Remove a user's achievement.", "$removeachievement [user] [name]");
    registerCommand(&Administration::removeCrates, std::initializer_list<std::string> { "removecrates", "rmcrates" }, "Remove a user's crates.", "$removecrates [user]");
    registerCommand(&Administration::removeStat, std::initializer_list<std::string> { "removestat", "rmstat" }, "Remove a user's stat.", "$removestat [user] [stat]");
    registerCommand(&Administration::resetCooldowns, "resetcd", "Reset a user's cooldowns.", "$resetcd [user]");
    registerCommand(&Administration::setCash, "setcash", "Set a user's cash.", "$setcash [user] [amount]");
    registerCommand(&Administration::setCrypto, "setcrypto", "Set a user's cryptocurrency amount. See $invest's help info for currently accepted currencies.", "$setcrypto [user] [crypto] [amount]");
    registerCommand(&Administration::setPrestige, "setprestige", "Set a user's prestige level.", "$setprestige [user] [level]");
    registerCommand(&Administration::setStat, "setstat", "Set a stat for a user.", "$setstat [user] [stat] [value]");
    registerCommand(&Administration::unlockAchievement, "unlockachievement", "Unlock an achievement for a user.", "$unlockachievement [user] [achievement]");
}

dpp::task<CommandResult> Administration::clearTextChannel(const ChannelTypeReader& channelRead)
{
    dpp::channel* channel = channelRead.topResult();
    co_await cluster->co_channel_delete(channel->id);
    co_await cluster->co_channel_create(*channel);
    co_return CommandResult::fromSuccess();
}

CommandResult Administration::drawPot()
{
    DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
    if (pot.endTime < RR::utility::unixTimeSecs())
        return CommandResult::fromError(Responses::PotEmpty);

    pot.endTime = 69;
    MongoManager::updatePot(pot);
    return CommandResult::fromSuccess(Responses::PotDrawing);
}

CommandResult Administration::removeAchievement(const UserTypeReader& userRead, const std::string& name)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (!std::erase_if(dbUser.achievements, [&name](auto& p) { return dpp::utility::iequals(p.first, name); }))
        return CommandResult::fromError(std::format(Responses::MissingAchievement, user->get_mention()));

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::RemovedAchievement, user->get_mention()));
}

CommandResult Administration::removeCrates(const UserTypeReader& userRead)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.crates.clear();

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::RemovedCrates, user->get_mention()));
}

CommandResult Administration::removeStat(const UserTypeReader& userRead, const std::string& stat)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (!std::erase_if(dbUser.stats, [&stat](auto& p) { return dpp::utility::iequals(p.first, stat); }))
        return CommandResult::fromError(std::format(Responses::MissingStat, user->get_mention()));

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::RemovedStat, user->get_mention()));
}

CommandResult Administration::resetCooldowns(const UserTypeReader& userRead)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.bullyCooldown = dbUser.chopCooldown = dbUser.cocaineRecoveryTime = dbUser.dailyCooldown =
    dbUser.dealCooldown = dbUser.digCooldown = dbUser.farmCooldown = dbUser.fishCooldown =
    dbUser.hackCooldown = dbUser.huntCooldown = dbUser.lootCooldown = dbUser.mineCooldown =
    dbUser.pacifistCooldown = dbUser.prestigeCooldown = dbUser.rapeCooldown = dbUser.robCooldown =
    dbUser.scavengeCooldown = dbUser.shootCooldown = dbUser.slaveryCooldown = dbUser.whoreCooldown = 0;

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::ResetCooldowns, user->get_mention()));
}

dpp::task<CommandResult> Administration::setCash(const UserTypeReader& userRead, long double amount)
{
    if (amount < 0)
        co_return CommandResult::fromError(Responses::NegativeCash);

    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        co_return CommandResult::fromError(Responses::UserIsBot);

    std::optional<dpp::guild_member> guildMember = RR::utility::findGuildMember(context->msg.guild_id, user->id);
    if (!guildMember)
        co_return CommandResult::fromError(Responses::GetUserFailed);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    co_await dbUser.setCashWithoutAdjustment(guildMember.value(), amount, cluster, context);

    MongoManager::updateUser(dbUser);
    co_return CommandResult::fromSuccess(std::format(Responses::SetCash, user->get_mention(), RR::utility::currencyToStr(amount)));
}

CommandResult Administration::setCrypto(const UserTypeReader& userRead, const std::string& crypto, long double amount)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

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
        return CommandResult::fromError(Responses::InvalidCrypto);

    std::string cryptoUpper;
    std::ranges::transform(crypto, std::back_inserter(cryptoUpper), toupper);

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::SetCrypto, user->get_mention(), cryptoUpper, amount));
}

CommandResult Administration::setPrestige(const UserTypeReader& userRead, int level)
{
    if (level < 0 || level > Constants::MaxPrestige)
        return CommandResult::fromError(Responses::InvalidPrestigeLevel);

    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.prestige = level;

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::SetPrestige, user->get_mention(), level));
}

CommandResult Administration::setStat(const UserTypeReader& userRead, const std::string& stat, const std::string& value)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.stats[stat] = value;

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess(std::format(Responses::SetStat, user->get_mention(), stat, value));
}

CommandResult Administration::unlockAchievement(const UserTypeReader& userRead, const std::string& name)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dbUser.unlockAchievement(name, context);

    MongoManager::updateUser(dbUser);
    return CommandResult::fromSuccess();
}
