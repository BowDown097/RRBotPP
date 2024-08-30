#pragma once
#include <dpp/coro/task.h>
#include <dpp/permissions.h>

namespace dpp
{
    class channel;
    class cluster;
    class embed_author;
    class guild;
    class guild_member;
    class message;
    class snowflake;
    class user;
}

namespace RR
{
    namespace utility
    {
        dpp::embed_author asEmbedAuthor(const dpp::guild_member& guildMember, const dpp::user* user, bool useNickname = true);
        dpp::channel* getDefaultChannel(const std::vector<dpp::snowflake>& channelIds, const dpp::user* botUser);
        std::string getDisplayAvatar(const dpp::guild_member& guildMember, const dpp::user* user);
        dpp::task<time_t> getJoinTime(dpp::cluster* cluster, dpp::guild* guild, dpp::snowflake userId);
        std::vector<dpp::snowflake> getLast14DaysMessages(const std::unordered_map<dpp::snowflake, dpp::message>& map);
        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions);
    }
}
