#pragma once
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
        dpp::embed_author asEmbedAuthor(const dpp::user* user, const dpp::guild_member* guildMember = nullptr,
                                        bool useNickname = true);
        dpp::channel* getDefaultChannel(const std::vector<dpp::snowflake>& channelIds, const dpp::user* botUser);
        std::string getDisplayAvatar(const dpp::user* user, const dpp::guild_member* guildMember = nullptr);
        std::vector<dpp::snowflake> getLast14DaysMessages(const std::unordered_map<dpp::snowflake, dpp::message>& map);
        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions);
    }
}
