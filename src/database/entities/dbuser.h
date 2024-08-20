#pragma once
#include "dbobject.h"
#include "utils/stringhash.h"
#include <dpp/coro/task.h>
#include <set>
#include <unordered_map>

namespace dpp { class channel; class cluster; class guild; class guild_member; class message_create_t; class user; }

struct DbUser : DbObject
{
    // ids
    int64_t guildId{};
    int64_t userId{};

    // cooldowns, time remaining, etc.
    int64_t blackHatEndTime{};
    int64_t bullyCooldown{};
    int64_t chopCooldown{};
    int64_t cocaineEndTime{};
    int64_t cocaineRecoveryTime{};
    int64_t dailyCooldown{};
    int64_t dealCooldown{};
    int64_t digCooldown{};
    int64_t farmCooldown{};
    int64_t fishCooldown{};
    int64_t hackCooldown{};
    int64_t huntCooldown{};
    int64_t lootCooldown{};
    int64_t messageCashCooldown{};
    int64_t mineCooldown{};
    int64_t pacifistCooldown{};
    int64_t prestigeCooldown{};
    int64_t rapeCooldown{};
    int64_t robCooldown{};
    int64_t scavengeCooldown{};
    int64_t shootCooldown{};
    int64_t skiMaskEndTime{};
    int64_t slaveryCooldown{};
    int64_t viagraEndTime{};
    int64_t whoreCooldown{};

    // "values" (currency, health, etc.)
    long double btc{};
    long double cash = 100;
    long double eth{};
    long double gamblingMultiplier = 1;
    int health = 100;
    long double ltc{};
    int prestige{};
    long double xrp{};

    // misc
    std::string gang;
    bool hasReachedAMilli{};
    bool usingSlots{};

    // complex
    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> achievements; // name, description
    std::unordered_map<std::string, int> ammo; // name, count
    std::unordered_map<std::string, int> collectibles; // name, count
    std::unordered_map<std::string, int> consumables; // name, count
    std::unordered_map<std::string, int> crates; // name, count
    std::set<std::string> pendingGangInvites;
    std::map<std::string, int64_t, std::less<>> perks; // name, duration
    std::unordered_map<std::string, std::string> stats; // name, value
    std::set<std::string, std::less<>> tools;
    std::unordered_map<std::string, int, string_hash, std::equal_to<>> usedConsumables;
    std::set<std::string, std::less<>> weapons;

    DbUser() = default;
    explicit DbUser(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;

    // this method is NOT safe. validate with resolveAbbreviation() before using!
    long double* getCrypto(std::string_view abbrev);

    std::unordered_map<std::string, int64_t&> constructCooldownMap();
    std::unordered_map<std::string, int64_t&> constructEndTimeMap();
    void mergeStat(const std::string& stat, const std::string& value);
    void mergeStats(const std::unordered_map<std::string, std::string>& statsToMerge);
    void modCooldown(int64_t& duration, const dpp::guild_member& member, bool speedDemon = true,
                     bool ranks = true, bool cocaine = true);
    dpp::task<void> setCash(const dpp::guild_member& member, long double amount,
                            dpp::cluster* cluster, const dpp::message_create_t* context = nullptr,
                            std::string message = "", bool showPrestigeMessage = true);
    dpp::task<void> setCashWithoutAdjustment(const dpp::guild_member& member, long double amount,
                                             dpp::cluster* cluster, const dpp::message_create_t* context = nullptr,
                                             const std::string& message = "");
    void unlockAchievement(std::string_view name, const dpp::message_create_t* context, const dpp::user* user = nullptr);
};
