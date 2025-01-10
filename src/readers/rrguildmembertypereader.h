#pragma once
#include "dppcmd/readers/guildmembertypereader.h"

namespace RR
{
    class guild_member_in final : public dppcmd::guild_member_in
    {
    public:
        dppcmd::type_reader_result read(dpp::cluster* cluster, const dpp::message_create_t* context,
                                        const std::string&) override;
    private:
        void add_results_by_global_name(const dpp::snowflake guild_id, const std::string& input);
        void add_results_by_nickname(const dpp::snowflake guild_id, const std::string& input);
        void add_results_by_username(const dpp::snowflake guild_id, const std::string& input);
    };
}
