#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class Economy : public dpp::module<Economy>
{
public:
    Economy();
private:
    dpp::command_result balance(const std::optional<dpp::guild_member>& memberOpt);
    dpp::command_result cooldowns(const std::optional<dpp::guild_member>& memberOpt);
    dpp::task<dpp::command_result> leaderboard(const std::optional<std::string>& currencyIn);
    dpp::command_result profile(std::optional<dpp::guild_member> memberOpt);
    dpp::command_result ranks();
    dpp::task<dpp::command_result> sauce(const dpp::guild_member& member, long double amount);
};
