#include "mongomanager.h"
#include "database/entities/dbban.h"
#include "database/entities/dbchill.h"
#include "database/entities/dbgang.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/entities/config/dbconfigranks.h"
#include "database/entities/config/dbconfigroles.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

using stream_document = bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

namespace MongoManager
{
    static mongocxx::instance* inst;
    static mongocxx::pool* pool;

    template<typename T>
    inline T caseOpt(bool caseInsensitive)
    {
        return caseInsensitive ? T().collation(stream_document() << "locale" << "en" << "strength" << 2 << finalize) : T();
    }

    mongocxx::database database()
    {
        mongocxx::pool::entry entry = pool->acquire();
        return entry->database("RR");
    }

    mongocxx::collection bans() { return database().collection("bans"); }
    mongocxx::collection chills() { return database().collection("chills"); }
    mongocxx::collection globalConfig() { return database().collection("globalconfig"); }
    mongocxx::collection pots() { return database().collection("pots"); }
    mongocxx::collection users() { return database().collection("users"); }

    mongocxx::collection channelConfigs() { return database().collection("channelconfigs"); }
    mongocxx::collection miscConfigs() { return database().collection("miscconfigs"); }
    mongocxx::collection rankConfigs() { return database().collection("rankconfigs"); }
    mongocxx::collection roleConfigs() { return database().collection("roleconfigs"); }

    // we have to specifically create the collection with support for case insensitive queries for some reason
    mongocxx::collection gangs()
    {
        if (database().has_collection("gangs"))
        {
            return database().collection("gangs");
        }
        else
        {
            return database().create_collection("gangs",
                stream_document() << "collation" << open_document
                    << "locale" << "en" << "strength" << 2
                << close_document << finalize);
        }
    }

    void initialize(std::string_view connectionString)
    {
        inst = new mongocxx::instance;
        mongocxx::options::client options;
        options.server_api_opts(mongocxx::options::server_api::version::k_version_1);
        mongocxx::uri uri(!connectionString.empty() ? connectionString : mongocxx::uri::k_default_uri);
        pool = new mongocxx::pool(uri, options);
    }

    void deleteGang(std::string_view name, int64_t guildId, bool caseInsensitive)
    {
        gangs().delete_one(
            stream_document() << "guildId" << guildId << "name" << name << finalize,
            caseOpt<mongocxx::options::delete_options>(caseInsensitive));
    }

    void deleteUser(int64_t userId, int64_t guildId)
    {
        users().delete_one(stream_document() << "userId" << userId << "guildId" << guildId << finalize);
    }

    void deleteChannelConfig(int64_t guildId)
    {
        channelConfigs().delete_one(stream_document() << "guildId" << guildId << finalize);
    }

    void deleteMiscConfig(int64_t guildId)
    {
        miscConfigs().delete_one(stream_document() << "guildId" << guildId << finalize);
    }

    void deleteRankConfig(int64_t guildId)
    {
        rankConfigs().delete_one(stream_document() << "guildId" << guildId << finalize);
    }

    void deleteRoleConfig(int64_t guildId)
    {
        roleConfigs().delete_one(stream_document() << "guildId" << guildId << finalize);
    }

    DbBan fetchBan(int64_t userId, int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = bans().find_one(stream_document()
            << "guildId" << guildId << "userId" << userId << finalize);
        if (doc)
            return DbBan(doc.value());

        DbBan newBan;
        newBan.guildId = guildId;
        newBan.userId = userId;

        bans().insert_one(newBan.toDocument());
        return newBan;
    }

    DbChill fetchChill(int64_t channelId, int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = chills().find_one(stream_document()
            << "channelId" << channelId << "guildId" << guildId << finalize);
        if (doc)
            return DbChill(doc.value());

        DbChill newChill;
        newChill.channelId = channelId;
        newChill.guildId = guildId;

        chills().insert_one(newChill.toDocument());
        return newChill;
    }

    DbGang fetchGang(std::string_view name, int64_t guildId, bool caseInsensitive)
    {
        std::optional<bsoncxx::document::value> doc = gangs().find_one(
            stream_document() << "guildId" << guildId << "name" << name << finalize,
            caseOpt<mongocxx::options::find>(caseInsensitive));
        return doc ? DbGang(doc.value()) : DbGang();
    }

    DbPot fetchPot(int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = pots().find_one(stream_document()
            << "guildId" << guildId << finalize);
        if (doc)
            return DbPot(doc.value());

        DbPot newPot;
        newPot.guildId = guildId;

        pots().insert_one(newPot.toDocument());
        return newPot;
    }

    DbUser fetchUser(int64_t userId, int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = users().find_one(stream_document()
            << "userId" << userId << "guildId" << guildId << finalize);
        if (doc)
            return DbUser(doc.value());

        DbUser newUser;
        newUser.guildId = guildId;
        newUser.userId = userId;

        users().insert_one(newUser.toDocument());
        return newUser;
    }

    DbConfigChannels fetchChannelConfig(int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = channelConfigs().find_one(stream_document()
            << "guildId" << guildId << finalize);
        if (doc)
            return DbConfigChannels(doc.value());

        DbConfigChannels newConfig;
        newConfig.guildId = guildId;

        channelConfigs().insert_one(newConfig.toDocument());
        return newConfig;
    }

    DbConfigGlobal fetchGlobalConfig()
    {
        std::optional<bsoncxx::document::value> doc = globalConfig().find_one({});
        if (doc)
            return DbConfigGlobal(doc.value());

        DbConfigGlobal newConfig;
        globalConfig().insert_one(newConfig.toDocument());
        return newConfig;
    }

    DbConfigMisc fetchMiscConfig(int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = miscConfigs().find_one(stream_document()
            << "guildId" << guildId << finalize);
        if (doc)
            return DbConfigMisc(doc.value());

        DbConfigMisc newConfig;
        newConfig.guildId = guildId;

        miscConfigs().insert_one(newConfig.toDocument());
        return newConfig;
    }

    DbConfigRanks fetchRankConfig(int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = rankConfigs().find_one(stream_document()
            << "guildId" << guildId << finalize);
        if (doc)
            return DbConfigRanks(doc.value());

        DbConfigRanks newConfig;
        newConfig.guildId = guildId;

        rankConfigs().insert_one(newConfig.toDocument());
        return newConfig;
    }

    DbConfigRoles fetchRoleConfig(int64_t guildId)
    {
        std::optional<bsoncxx::document::value> doc = roleConfigs().find_one(stream_document()
            << "guildId" << guildId << finalize);
        if (doc)
            return DbConfigRoles(doc.value());

        DbConfigRoles newConfig;
        newConfig.guildId = guildId;

        roleConfigs().insert_one(newConfig.toDocument());
        return newConfig;
    }

    void updateBan(const DbBan& ban)
    {
        bans().replace_one(
            stream_document() << "guildId" << ban.guildId << "userId" << ban.userId << finalize,
            ban.toDocument());
    }

    void updateChill(const DbChill& chill)
    {
        chills().replace_one(
            stream_document() << "channelId" << chill.channelId << "guildId" << chill.guildId << finalize,
            chill.toDocument());
    }

    void updateGang(const DbGang& gang, std::string_view nameOverride)
    {
        if (nameOverride.empty())
        {
            gangs().replace_one(
                stream_document() << "guildId" << gang.guildId << "name" << gang.name << finalize,
                gang.toDocument());
        }
        else
        {
            gangs().replace_one(
                stream_document() << "guildId" << gang.guildId << "name" << nameOverride << finalize,
                gang.toDocument());
        }
    }

    void updatePot(const DbPot& pot)
    {
        pots().replace_one(stream_document() << "guildId" << pot.guildId << finalize, pot.toDocument());
    }

    void updateUser(const DbUser& user)
    {
        users().replace_one(
            stream_document() << "userId" << user.userId << "guildId" << user.guildId << finalize,
            user.toDocument());
    }

    void updateChannelConfig(const DbConfigChannels& config)
    {
        channelConfigs().replace_one(
            stream_document() << "guildId" << config.guildId << finalize,
            config.toDocument());
    }

    void updateGlobalConfig(const DbConfigGlobal& config)
    {
        globalConfig().replace_one({}, config.toDocument());
    }

    void updateMiscConfig(const DbConfigMisc& config)
    {
        miscConfigs().replace_one(
            stream_document() << "guildId" << config.guildId << finalize,
            config.toDocument());
    }

    void updateRankConfig(const DbConfigRanks& config)
    {
        rankConfigs().replace_one(
            stream_document() << "guildId" << config.guildId << finalize,
            config.toDocument());
    }

    void updateRoleConfig(const DbConfigRoles& config)
    {
        roleConfigs().replace_one(
            stream_document() << "guildId" << config.guildId << finalize,
            config.toDocument());
    }
}
