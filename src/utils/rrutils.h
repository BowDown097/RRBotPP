#ifndef RRUTILS_H
#define RRUTILS_H
#include <dpp/coro/task.h>
#include <dpp/restresults.h>
#include <iostream>

namespace RR
{
    namespace utility
    {
        template<typename T>
        dpp::task<T> co_get(dpp::async<dpp::confirmation_callback_t>&& task)
        {
            dpp::confirmation_callback_t result = co_await task;
            if (!result.is_error())
                co_return result.get<T>();
            else
                std::cout << result.get_error().human_readable << std::endl;
        }

        dpp::embed_author asEmbedAuthor(const dpp::guild_member& guildMember, const dpp::user* user,
                                        bool useNickname = true);
        std::optional<dpp::guild_member> findGuildMember(const dpp::snowflake guildId, const dpp::snowflake userId);
        std::string formatPair(const std::pair<std::string, std::string>& pair);
        std::string formatTimestamp(long timestamp);
        std::string getDisplayAvatar(const dpp::guild_member& guildMember, const dpp::user* user);
        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions);
        std::string sanitizeString(const std::string& str);
        void strReplace(std::string& str, std::string_view from, std::string_view to);
        long unixTimeSecs(long offset = 0);
    }
}

#endif // RRUTILS_H
