#ifndef MONGOMANAGER_H
#define MONGOMANAGER_H
#include <cstdint>
#include <mongocxx/collection-fwd.hpp>
#include <string_view>

class DbBan;
class DbChill;
class DbGang;
class DbPot;
class DbUser;
class DbConfigChannels;
class DbConfigGlobal;
class DbConfigMisc;
class DbConfigRanks;
class DbConfigRoles;

namespace MongoManager
{
    void initialize(std::string_view connectionString);

    mongocxx::collection bans();
    mongocxx::collection chills();
    mongocxx::collection gangs();
    mongocxx::collection globalConfig();
    mongocxx::collection pots();
    mongocxx::collection users();

    mongocxx::collection channelConfigs();
    mongocxx::collection miscConfigs();
    mongocxx::collection rankConfigs();
    mongocxx::collection roleConfigs();

    DbBan fetchBan(int64_t userId, int64_t guildId);
    DbChill fetchChill(int64_t channelId, int64_t guildId);
    DbGang fetchGang(std::string_view name, int64_t guildId);
    DbPot fetchPot(int64_t guildId);
    DbUser fetchUser(int64_t userId, int64_t guildId);

    DbConfigChannels fetchChannelConfig(int64_t guildId);
    DbConfigGlobal fetchGlobalConfig();
    DbConfigMisc fetchMiscConfig(int64_t guildId);
    DbConfigRanks fetchRankConfig(int64_t guildId);
    DbConfigRoles fetchRoleConfig(int64_t guildId);

    void updateBan(const DbBan& ban);
    void updateChill(const DbChill& chill);
    void updateGang(const DbGang& gang);
    void updatePot(const DbPot& pot);
    void updateUser(const DbUser& user);

    void updateChannelConfig(const DbConfigChannels& config);
    void updateGlobalConfig(const DbConfigGlobal& config);
    void updateMiscConfig(const DbConfigMisc& config);
    void updateRankConfig(const DbConfigRanks& config);
    void updateRoleConfig(const DbConfigRoles& config);

    void deleteUser(int64_t userId, int64_t guildId);

    void deleteChannelConfig(int64_t guildId);
    void deleteMiscConfig(int64_t guildId);
    void deleteRankConfig(int64_t guildId);
    void deleteRoleConfig(int64_t guildId);
}

#endif // MONGOMANAGER_H
