#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Economy : public dppcmd::module<Economy>
{
public:
    Economy();
private:
    dppcmd::command_result balance(const std::optional<dpp::guild_member>& memberOpt);
    dppcmd::command_result cooldowns(const std::optional<dpp::guild_member>& memberOpt);
    dpp::task<dppcmd::command_result> leaderboard(const std::optional<std::string>& currencyIn);
    dppcmd::command_result profile(std::optional<dpp::guild_member> memberOpt);
    dppcmd::command_result ranks();
    dpp::task<dppcmd::command_result> sauce(const dpp::guild_member& member, long double amount);
};
