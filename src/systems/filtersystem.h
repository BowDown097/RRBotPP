#pragma once
#include <dpp/coro/task.h>

namespace dpp
{
    class cluster;
    class message;
}

namespace FilterSystem
{
    bool containsFilteredWord(const dpp::snowflake& guildId, std::string_view input);
    dpp::task<void> doFilteredWordCheck(const dpp::message& message, dpp::cluster* cluster);
    dpp::task<void> doInviteCheck(const dpp::message& message, dpp::cluster* cluster);
    dpp::task<void> doScamCheck(const dpp::message& message, dpp::cluster* cluster);
}
