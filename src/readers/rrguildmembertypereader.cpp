#include "rrguildmembertypereader.h"
#include "data/responses.h"
#include "dppcmd/utils/join.h"
#include "utils/strings.h"
#include <dpp/cache.h>
#include <dpp/dispatcher.h>

namespace RR
{
    dppcmd::type_reader_result guild_member_in::read(dpp::cluster* cluster,
        const dpp::message_create_t* context, const std::string& input)
    {
        add_results_by_mention(context->msg.guild_id, input); // weight: 1.0
        add_results_by_id(context->msg.guild_id, input); // weight: 0.9
        add_results_by_global_name(context->msg.guild_id, input); // weight: 0.7-0.8
        add_results_by_username(context->msg.guild_id, input); // weight: 0.7-0.8
        add_results_by_nickname(context->msg.guild_id, input); // weight: 0.7-0.8

        if (results().size() == 1)
        {
            return dppcmd::type_reader_result::from_success();
        }
        else if (results().size() > 1)
        {
            std::string response = std::format(Responses::GuildMemberAmbiguous,
                results().size(), dppcmd::utility::join(results(), ", ", [](const auto& v) { return v->get_mention(); }));
            return dppcmd::type_reader_result::from_error(dppcmd::command_error::multiple_matches, response);
        }
        else
        {
            return dppcmd::type_reader_result::from_error(dppcmd::command_error::object_not_found, "User not found.");
        }
    }

    void guild_member_in::add_results_by_global_name(const dpp::snowflake guild_id, const std::string& input)
    {
        if (const dpp::guild* guild = dpp::find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (const dpp::user* user = member.get_user(); utility::iStartsWith(user->global_name, input))
                    if (id_set.insert(user->id).second)
                        add_result(member, user->global_name == input ? 0.8f : 0.7f);
    }

    void guild_member_in::add_results_by_nickname(const dpp::snowflake guild_id, const std::string& input)
    {
        if (const dpp::guild* guild = dpp::find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (std::string nickname = member.get_nickname(); utility::iStartsWith(nickname, input))
                    if (id_set.insert(member.user_id).second)
                        add_result(member, nickname == input ? 0.8f : 0.7f);
    }

    void guild_member_in::add_results_by_username(const dpp::snowflake guild_id, const std::string& input)
    {
        if (const dpp::guild* guild = dpp::find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (const dpp::user* user = member.get_user(); utility::iStartsWith(user->username, input))
                    if (id_set.insert(user->id).second)
                        add_result(member, user->username == input ? 0.8f : 0.7f);
    }
}
