#include "dbuser.h"
#include "data/constants.h"
#include "database/entities/config/dbconfigranks.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "dpp-command-handler/utils/strings.h"
#include "utils/ld.h"
#include "utils/strings.h"
#include "utils/timestamp.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <dpp/cluster.h>
#include <dpp/colors.h>

DbUser::DbUser(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    userId = bsoncxx_get_or_default(doc["userId"], int64);

    blackHatEndTime = bsoncxx_get_or_default(doc["blackHatEndTime"], int64);
    bullyCooldown = bsoncxx_get_or_default(doc["bullyCooldown"], int64);
    chopCooldown = bsoncxx_get_or_default(doc["chopCooldown"], int64);
    cocaineEndTime = bsoncxx_get_or_default(doc["cocaineEndTime"], int64);
    cocaineRecoveryTime = bsoncxx_get_or_default(doc["cocaineRecoveryTime"], int64);
    dailyCooldown = bsoncxx_get_or_default(doc["dailyCooldown"], int64);
    dealCooldown = bsoncxx_get_or_default(doc["dealCooldown"], int64);
    digCooldown = bsoncxx_get_or_default(doc["digCooldown"], int64);
    farmCooldown = bsoncxx_get_or_default(doc["farmCooldown"], int64);
    fishCooldown = bsoncxx_get_or_default(doc["fishCooldown"], int64);
    hackCooldown = bsoncxx_get_or_default(doc["hackCooldown"], int64);
    huntCooldown = bsoncxx_get_or_default(doc["huntCooldown"], int64);
    lootCooldown = bsoncxx_get_or_default(doc["lootCooldown"], int64);
    messageCashCooldown = bsoncxx_get_or_default(doc["messageCashCooldown"], int64);
    mineCooldown = bsoncxx_get_or_default(doc["mineCooldown"], int64);
    pacifistCooldown = bsoncxx_get_or_default(doc["pacifistCooldown"], int64);
    prestigeCooldown = bsoncxx_get_or_default(doc["prestigeCooldown"], int64);
    rapeCooldown = bsoncxx_get_or_default(doc["rapeCooldown"], int64);
    robCooldown = bsoncxx_get_or_default(doc["robCooldown"], int64);
    scavengeCooldown = bsoncxx_get_or_default(doc["scavengeCooldown"], int64);
    shootCooldown = bsoncxx_get_or_default(doc["shootCooldown"], int64);
    skiMaskEndTime = bsoncxx_get_or_default(doc["skiMaskEndTime"], int64);
    slaveryCooldown = bsoncxx_get_or_default(doc["slaveryCooldown"], int64);
    viagraEndTime = bsoncxx_get_or_default(doc["viagraEndTime"], int64);
    whoreCooldown = bsoncxx_get_or_default(doc["whoreCooldown"], int64);

    btc = RR::utility::get_long_double(doc["btc"]);
    cash = RR::utility::get_long_double(doc["cash"]);
    eth = RR::utility::get_long_double(doc["eth"]);
    gamblingMultiplier = RR::utility::get_long_double(doc["gamblingMultiplier"]);
    health = bsoncxx_get_or_default(doc["health"], int32);
    ltc = RR::utility::get_long_double(doc["ltc"]);
    prestige = bsoncxx_get_or_default(doc["prestige"], int32);
    xrp = RR::utility::get_long_double(doc["xrp"]);

    gang = bsoncxx_get_value_or_default(doc["gang"], string);
    hasReachedAMilli = bsoncxx_get_or_default(doc["hasReachedAMilli"], bool);
    usingSlots = bsoncxx_get_or_default(doc["usingSlots"], bool);

    bsoncxx_elem_to_map(doc["achievements"], achievements, string);
    bsoncxx_elem_to_map(doc["ammo"], ammo, int32);
    bsoncxx_elem_to_map(doc["collectibles"], collectibles, int32);
    bsoncxx_elem_to_map(doc["consumables"], consumables, int32);
    bsoncxx_elem_to_map(doc["crates"], crates, int32);
    bsoncxx_elem_to_map(doc["perks"], perks, int64);
    bsoncxx_elem_to_map(doc["stats"], stats, string);
    bsoncxx_elem_to_map(doc["usedConsumables"], usedConsumables, int32);

    bsoncxx_elem_to_set(doc["pendingGangInvites"], pendingGangInvites, string);
    bsoncxx_elem_to_set(doc["tools"], tools, string);
    bsoncxx_elem_to_set(doc["weapons"], weapons, string);
}

bsoncxx::document::value DbUser::toDocument() const
{
    bsoncxx::builder::stream::document achievementsDoc;
    bsoncxx_stream_map_into(achievements, achievementsDoc);

    bsoncxx::builder::stream::document ammoDoc;
    bsoncxx_stream_map_into(ammo, ammoDoc);

    bsoncxx::builder::stream::document collectiblesDoc;
    bsoncxx_stream_map_into(collectibles, collectiblesDoc);

    bsoncxx::builder::stream::document consumablesDoc;
    bsoncxx_stream_map_into(consumables, consumablesDoc);

    bsoncxx::builder::stream::document cratesDoc;
    bsoncxx_stream_map_into(crates, cratesDoc);

    bsoncxx::builder::stream::array pendingGangInvitesArr;
    bsoncxx_stream_array_into(pendingGangInvites, pendingGangInvitesArr);

    bsoncxx::builder::stream::document perksDoc;
    bsoncxx_stream_map_into(perks, perksDoc);

    bsoncxx::builder::stream::document statsDoc;
    bsoncxx_stream_map_into(stats, statsDoc);

    bsoncxx::builder::stream::array toolsArr;
    bsoncxx_stream_array_into(tools, toolsArr);

    bsoncxx::builder::stream::document usedConsumablesDoc;
    bsoncxx_stream_map_into(usedConsumables, usedConsumablesDoc);

    bsoncxx::builder::stream::array weaponsArr;
    bsoncxx_stream_array_into(weapons, weaponsArr);

    return bsoncxx::builder::stream::document()
           << "achievements" << achievementsDoc
           << "ammo" << ammoDoc
           << "blackHatEndTime" << blackHatEndTime
           << "btc" << RR::utility::put_long_double(btc)
           << "bullyCooldown" << bullyCooldown
           << "cash" << RR::utility::put_long_double(cash)
           << "chopCooldown" << chopCooldown
           << "cocaineEndTime" << cocaineEndTime
           << "cocaineRecoveryTime" << cocaineRecoveryTime
           << "collectibles" << collectiblesDoc
           << "consumables" << consumablesDoc
           << "crates" << cratesDoc
           << "dailyCooldown" << dailyCooldown
           << "dealCooldown" << dealCooldown
           << "digCooldown" << digCooldown
           << "eth" << RR::utility::put_long_double(eth)
           << "farmCooldown" << farmCooldown
           << "fishCooldown" << fishCooldown
           << "gamblingMultiplier" << RR::utility::put_long_double(gamblingMultiplier)
           << "gang" << gang
           << "guildId" << guildId
           << "hackCooldown" << hackCooldown
           << "hasReachedAMilli" << hasReachedAMilli
           << "health" << health
           << "huntCooldown" << huntCooldown
           << "lootCooldown" << lootCooldown
           << "ltc" << RR::utility::put_long_double(ltc)
           << "messageCashCooldown" << messageCashCooldown
           << "mineCooldown" << mineCooldown
           << "pacifistCooldown" << pacifistCooldown
           << "pendingGangInvites" << pendingGangInvitesArr
           << "perks" << perksDoc
           << "prestige" << prestige
           << "prestigeCooldown" << prestigeCooldown
           << "rapeCooldown" << rapeCooldown
           << "robCooldown" << robCooldown
           << "scavengeCooldown" << scavengeCooldown
           << "shootCooldown" << shootCooldown
           << "skiMaskEndTime" << skiMaskEndTime
           << "slaveryCooldown" << slaveryCooldown
           << "stats" << statsDoc
           << "tools" << toolsArr
           << "usedConsumables" << usedConsumablesDoc
           << "userId" << userId
           << "usingSlots" << usingSlots
           << "viagraEndTime" << viagraEndTime
           << "weapons" << weaponsArr
           << "whoreCooldown" << whoreCooldown
           << "xrp" << RR::utility::put_long_double(xrp)
           << bsoncxx::builder::stream::finalize;
}

std::unordered_map<std::string, int64_t&> DbUser::constructCooldownMap()
{
    return {
        { "Bully", bullyCooldown },
        { "Chop", chopCooldown },
        { "Daily", dailyCooldown },
        { "Deal", dealCooldown },
        { "Dig", digCooldown },
        { "Farm", farmCooldown },
        { "Fish", fishCooldown },
        { "Hack", hackCooldown },
        { "Hunt", huntCooldown },
        { "Loot", lootCooldown },
        { "Mine", mineCooldown },
        { "OD Recovery", cocaineRecoveryTime },
        { "Pacifist", pacifistCooldown },
        { "Prestige", prestigeCooldown },
        { "Rape", rapeCooldown },
        { "Rob", robCooldown },
        { "Scavenge", scavengeCooldown },
        { "Shoot", shootCooldown },
        { "Slavery", slaveryCooldown },
        { "Whore", whoreCooldown }
    };
}

long double* DbUser::getCrypto(std::string_view abbrev)
{
    if (abbrev == "btc")
        return std::addressof(btc);
    else if (abbrev == "eth")
        return std::addressof(eth);
    else if (abbrev == "ltc")
        return std::addressof(ltc);
    else if (abbrev == "xrp")
        return std::addressof(xrp);
    return nullptr;
}

void DbUser::mergeStat(const std::string& stat, const std::string& value)
{
    mergeStats({ { stat, value } });
}

void DbUser::mergeStats(const std::unordered_map<std::string, std::string>& statsToMerge)
{
    for (const auto& [name, value] : statsToMerge)
    {
        if (auto it = stats.find(name); it != stats.end())
        {
            if (std::optional<long double> toAdd = RR::utility::str2curr(value),
                oldValue = RR::utility::str2curr(it->second); toAdd && oldValue)
            {
                stats[name] = RR::utility::curr2str(oldValue.value() + toAdd.value());
            }

            try
            {
                long double toAdd = dpp::utility::lexical_cast<long double>(value);
                long double oldValue = dpp::utility::lexical_cast<long double>(value);
                std::string finalValue = std::format("{:.4f}", oldValue + toAdd);
                stats[name] = RR::utility::trimZeros(finalValue);
            }
            catch (const dpp::utility::bad_lexical_cast&) {}
        }
        else
        {
            stats.emplace(name, value);
        }
    }
}

void DbUser::modCooldown(int64_t& duration, const dpp::guild_member& member, bool speedDemon, bool ranks, bool cocaine)
{
    duration = RR::utility::unixTimestamp(duration);
    // speed demon cooldown reducer
    if (speedDemon && this->perks.contains("Speed Demon"))
        duration *= 0.85;
    // 4th rank cooldown reducer
    if (ranks)
    {
        DbConfigRanks ranksConfig = MongoManager::fetchRankConfig(member.guild_id);
        if (auto it = ranksConfig.ids.find(4); it != ranksConfig.ids.end())
            if (std::ranges::contains(member.get_roles(), dpp::snowflake(it->second)))
                duration *= 0.8;
    }
    // cocaine cooldown reducer
    if (cocaine && this->usedConsumables["Cocaine"] > 0)
        duration *= std::pow(0.9, usedConsumables["Cocaine"]);
}

dpp::task<void> DbUser::setCash(const dpp::guild_member& member, long double amount,
                                dpp::cluster* cluster, const dpp::message_create_t* context,
                                std::string message, bool showPrestigeMessage)
{
    if (member.get_user()->is_bot())
        co_return;
    if (amount < 0)
        amount = 0;

    amount *= Constants::CashMultiplier;

    long double difference = amount - this->cash;
    if (this->prestige > 0 && difference > 0)
    {
        long double prestigeCash = difference * 0.20 * this->prestige;
        difference += prestigeCash;
        if (showPrestigeMessage)
            message += std::format("\n*(+{} from Prestige)*", RR::utility::curr2str(prestigeCash));
    }

    co_await setCashWithoutAdjustment(member, this->cash + difference, cluster, context, message);
}

dpp::task<void> DbUser::setCashWithoutAdjustment(const dpp::guild_member& member, long double amount,
                                                 dpp::cluster* cluster, const dpp::message_create_t* context,
                                                 const std::string& message)
{
    this->cash = amount;
    dpp::guild* guild = dpp::find_guild(member.guild_id);

    if (!message.empty() && context)
        context->reply(message);

    DbConfigRanks ranks = MongoManager::fetchRankConfig(member.guild_id);
    for (const auto& [level, cost] : ranks.costs)
    {
        dpp::snowflake roleId = ranks.ids[level];
        if (!std::ranges::contains(guild->roles, roleId))
            co_return;

        bool hasRole = std::ranges::contains(member.get_roles(), roleId);
        long double neededCash = cost * (1 + 0.5 * this->prestige);

        if (this->cash >= neededCash && !hasRole)
            co_await cluster->co_guild_member_add_role(guild->id, member.user_id, roleId);
        else if (this->cash <= neededCash && hasRole)
            co_await cluster->co_guild_member_remove_role(guild->id, member.user_id, roleId);
    }
}

void DbUser::unlockAchievement(const std::string& name, const dpp::message_create_t* context, const dpp::user* user)
{
    if (std::ranges::any_of(this->achievements, [&name](const auto& p) { return dpp::utility::iequals(p.first, name); }))
        return;

    const Achievement* ach = std::ranges::find_if(Constants::DefaultAchievements, [&name](const Achievement& a) {
        return dpp::utility::iequals(a.name(), name);
    });

    if (!ach)
        return;

    achievements.emplace(ach->name(), ach->description());

    std::string embedDescription = std::format(
        "GG {}, you unlocked an achievement.\n**{}**: {}",
        user ? user->get_mention() : context->msg.author.get_mention(), ach->name(), ach->description());

    if (ach->reward() > 0)
    {
        this->cash += ach->reward();
        embedDescription += "\nReward: " + RR::utility::curr2str(ach->reward());
    }

    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_title("Achievement Get!")
       .set_description(embedDescription);
    context->reply(dpp::message(context->msg.channel_id, embed));

    bool hasGamblingAchievements = std::ranges::all_of(Constants::GamblingAchievementNames, [this](const char* a) {
        return this->achievements.contains(a);
    });

    if (this->gamblingMultiplier == 1.0 && hasGamblingAchievements)
    {
        this->gamblingMultiplier = 1.1;
        context->reply("Congratulations! You've acquired every gambling achievement. Enjoy this **1.1x gambling multiplier**!");
    }
}
