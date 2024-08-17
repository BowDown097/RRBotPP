#pragma once
#include "dpp-command-handler/readers/guildmembertypereader.h"

namespace RR
{
    class guild_member_in : public dpp::guild_member_in
    {
    public:
        dpp::type_reader_result read(dpp::cluster* cluster, const dpp::message_create_t* context,
                                     std::string_view input) override;
    private:
        void add_results_by_global_name(const dpp::snowflake guild_id, std::string_view input);
        void add_results_by_nickname(const dpp::snowflake guild_id, std::string_view input);
        void add_results_by_username(const dpp::snowflake guild_id, std::string_view input);
    };
}
