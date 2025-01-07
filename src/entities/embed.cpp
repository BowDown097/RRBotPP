#include "embed.h"
#include "dppcmd/extensions/cache.h"
#include "utils/dpp.h"
#include <dpp/cache.h>

namespace RR
{
    embed& embed::add_separator()
    {
        dpp::embed::add_field("\u200b", "\u200b");
        return *this;
    }

    embed& embed::add_string_field(const std::string& name, const std::string& value, bool is_inline,
                                   bool show_if_not_available, const std::string& default_value)
    {
        if (!value.empty() || show_if_not_available)
            dpp::embed::add_field(name, value.empty() ? default_value : value, is_inline);
        return *this;
    }

    embed& embed::set_author(const dpp::guild_member& member, dpp::user* user)
    {
        if (user || (user = dpp::find_user(member.user_id)))
            dpp::embed::set_author(RR::utility::asEmbedAuthor(user, &member, false));
        return *this;
    }

    embed& embed::set_author(const dpp::guild_member& member, const dpp::user& user)
    {
        dpp::embed::set_author(RR::utility::asEmbedAuthor(&user, &member, false));
        return *this;
    }

    embed& embed::set_author(dpp::snowflake guild_id, dpp::snowflake user_id)
    {
        if (auto member = dppcmd::find_guild_member_opt(guild_id, user_id))
        {
            if (const dpp::user* user = member->get_user())
            {
                dpp::embed::set_author(RR::utility::asEmbedAuthor(user, &member.value(), false));
            }
        }
        else if (const dpp::user* user = dpp::find_user(user_id))
        {
            dpp::embed::set_author(RR::utility::asEmbedAuthor(user));
        }
        return *this;
    }
}
