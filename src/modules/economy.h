#pragma once
#include "dpp-command-handler/modules/module.h"
#include "readers/cashtypereader.h"
#include "readers/rrguildmembertypereader.h"

class Economy : public dpp::module<Economy>
{
public:
    Economy();
private:
    dpp::command_result balance(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::command_result cooldowns(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::task<dpp::command_result> leaderboard(const std::optional<std::string>& currencyIn);
    dpp::command_result profile(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::command_result ranks();
    dpp::task<dpp::command_result> sauce(const RR::guild_member_in& memberIn, const cash_in& amountIn);
};
