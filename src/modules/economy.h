#pragma once
#include "dpp-command-handler/modules/module.h"
#include "dpp-command-handler/readers/guildmembertypereader.h"
#include "dpp-command-handler/readers/usertypereader.h"
#include "readers/cashtypereader.h"

class Economy : public dpp::module<Economy>
{
public:
    Economy();
private:
    dpp::command_result balance(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result cooldowns(const std::optional<dpp::user_in>& userOpt);
    dpp::task<dpp::command_result> leaderboard(const std::optional<std::string>& currencyIn);
    dpp::command_result profile(const std::optional<dpp::guild_member_in>& memberOpt);
    dpp::command_result ranks();
    dpp::task<dpp::command_result> sauce(const dpp::guild_member_in& memberIn, const cash_in& amountIn);
};
