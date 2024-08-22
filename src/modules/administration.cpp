#include "administration.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/utils/strings.h"
#include "investments.h"
#include "systems/itemsystem.h"
#include "utils/ld.h"
#include "utils/strings.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>
#include <mongocxx/collection.hpp>

Administration::Administration() : dpp::module<Administration>("Administration", "Commands for admin stuff. Whether you wanna screw with the economy or fuck someone over, I'm sure you'll have fun. However, you'll need to have a very high role to have all this fun. Sorry!")
{
    register_command(&Administration::clearTextChannel, "cleartextchannel", "Deletes and recreates a text channel, effectively wiping its messages.", "$cleartextchannel [channel]");
    register_command(&Administration::drawPot, "drawpot", "Draw the pot before it ends.");
    register_command(&Administration::giveItem, "giveitem", "Give a user an item.", "$giveitem [user] [item]");
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

dpp::task<dpp::command_result> Administration::clearTextChannel(dpp::channel* channel)
{
    co_await cluster->co_channel_delete(channel->id);
    co_await cluster->co_channel_create(*channel);
    co_return dpp::command_result::from_success();
}

dpp::command_result Administration::drawPot()
{
    DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
    if (pot.endTime < RR::utility::unixTimestamp())
        return dpp::command_result::from_error(Responses::PotEmpty);

    pot.endTime = 69;
    MongoManager::updatePot(pot);
    return dpp::command_result::from_success(Responses::PotDrawing);
}

dpp::command_result Administration::giveItem(const dpp::guild_member& member, const dpp::remainder<std::string>& itemIn)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    if (const Item* item = ItemSystem::getItem(*itemIn))
    {
        DbUser dbUser = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);

        if (dynamic_cast<const Ammo*>(item))
        {
            dbUser.ammo[std::string(item->name())]++;
        }
        else if (dynamic_cast<const Collectible*>(item))
        {
            dbUser.collectibles[std::string(item->name())]++;
        }
        else if (dynamic_cast<const Consumable*>(item))
        {
            dbUser.consumables[std::string(item->name())]++;
        }
        else if (dynamic_cast<const Crate*>(item))
        {
            dbUser.crates[std::string(item->name())]++;
        }
        else if (const Perk* perk = dynamic_cast<const Perk*>(item))
        {
            if (auto res = dbUser.perks.emplace(perk->name(), perk->duration()); !res.second)
                return dpp::command_result::from_error(std::format(Responses::UserAlreadyHasThing, member.get_mention(), perk->name()));
        }
        else if (dynamic_cast<const Tool*>(item))
        {
            if (auto res = dbUser.tools.emplace(item->name()); !res.second)
                return dpp::command_result::from_error(std::format(Responses::UserAlreadyHasThing, member.get_mention(), item->name()));
        }
        else if (dynamic_cast<const Weapon*>(item))
        {
            if (auto res = dbUser.weapons.emplace(item->name()); !res.second)
                return dpp::command_result::from_error(std::format(Responses::UserAlreadyHasThing, member.get_mention(), item->name()));
        }

        MongoManager::updateUser(dbUser);
        return dpp::command_result::from_success(std::format(Responses::GaveUserItem, member.get_mention(), item->name()));
    }

    return dpp::command_result::from_error(Responses::NotAnItem);
}

dpp::command_result Administration::removeAchievement(const dpp::guild_member& member, const dpp::remainder<std::string>& name)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (!std::erase_if(dbUser.achievements, [&name](auto& p) { return dpp::utility::iequals(p.first, *name); }))
        return dpp::command_result::from_error(std::format(Responses::MissingAchievement, member.get_mention()));

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedAchievement, member.get_mention()));
}

dpp::command_result Administration::removeCrates(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    dbUser.crates.clear();

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedCrates, member.get_mention()));
}

dpp::command_result Administration::removeStat(const dpp::guild_member& member, const dpp::remainder<std::string>& stat)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (!std::erase_if(dbUser.stats, [&stat](auto& p) { return dpp::utility::iequals(p.first, *stat); }))
        return dpp::command_result::from_error(std::format(Responses::MissingStat, member.get_mention()));

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::RemovedStat, member.get_mention()));
}

dpp::command_result Administration::resetCooldowns(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    dbUser.bullyCooldown = dbUser.chopCooldown = dbUser.cocaineRecoveryTime = dbUser.dailyCooldown =
    dbUser.dealCooldown = dbUser.digCooldown = dbUser.farmCooldown = dbUser.fishCooldown =
    dbUser.hackCooldown = dbUser.huntCooldown = dbUser.lootCooldown = dbUser.mineCooldown =
    dbUser.pacifistCooldown = dbUser.prestigeCooldown = dbUser.rapeCooldown = dbUser.robCooldown =
    dbUser.scavengeCooldown = dbUser.shootCooldown = dbUser.slaveryCooldown = dbUser.whoreCooldown = 0;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::ResetCooldowns, member.get_mention()));
}

dpp::task<dpp::command_result> Administration::setCash(const dpp::guild_member& member, long double amount)
{
    if (amount < 0)
        co_return dpp::command_result::from_error(Responses::NegativeCash);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    co_await dbUser.setCashWithoutAdjustment(member, amount, cluster);

    MongoManager::updateUser(dbUser);
    co_return dpp::command_result::from_success(std::format(Responses::SetCash,
        member.get_mention(), RR::utility::cash2str(amount)));
}

dpp::command_result Administration::setCrypto(const dpp::guild_member& member, const std::string& crypto, long double amount)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    std::string abbrev = Investments::resolveAbbreviation(crypto);
    if (abbrev.empty())
        return dpp::command_result::from_error(Responses::InvalidCurrency);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    *dbUser.getCrypto(abbrev) = amount = RR::utility::round(amount, 4);

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetCrypto,
        member.get_mention(), RR::utility::toUpper(crypto), RR::utility::roundAsStr(amount, 4)));
}

dpp::command_result Administration::setPrestige(const dpp::guild_member& member, int level)
{
    if (level < 0 || level > Constants::MaxPrestige)
        return dpp::command_result::from_error(Responses::InvalidPrestigeLevel);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    dbUser.prestige = level;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetPrestige, member.get_mention(), level));
}

dpp::command_result Administration::setStat(const dpp::guild_member& member, const std::string& stat,
                                            const dpp::remainder<std::string>& value)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    dbUser.stats[stat] = *value;

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success(std::format(Responses::SetStat, member.get_mention(), stat, *value));
}

dpp::command_result Administration::unlockAchievement(const dpp::guild_member& member, const dpp::remainder<std::string>& name)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    dbUser.unlockAchievement(*name, context);

    MongoManager::updateUser(dbUser);
    return dpp::command_result::from_success();
}
