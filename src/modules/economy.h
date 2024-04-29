#ifndef ECONOMY_H
#define ECONOMY_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"
#include "readers/cashtypereader.h"

class Economy : public dpp::module_base
{
public:
    Economy();
    MODULE_SETUP(Economy)
private:
    dpp::command_result balance(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result cooldowns(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result profile(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result ranks();
    dpp::task<dpp::command_result> sauce(const dpp::user_in& userIn, const cash_in& amountIn);
};

#endif // ECONOMY_H
