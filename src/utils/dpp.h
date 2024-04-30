#pragma once
#include <dpp/permissions.h>

namespace dpp { class embed_author; class guild_member; class user; }

namespace RR
{
    namespace utility
    {
        dpp::embed_author asEmbedAuthor(const dpp::guild_member& guildMember, const dpp::user* user,
                                        bool useNickname = true);
        std::string getDisplayAvatar(const dpp::guild_member& guildMember, const dpp::user* user);
        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions);
    }
}
