#pragma once
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/guildmembertypereader.h"
#include "readers/cashtypereader.h"

class Investments : public dpp::module<Investments>
{
public:
    Investments();
    static dpp::task<std::optional<long double>> queryCryptoValue(std::string_view crypto, dpp::cluster* cluster);
    static std::string resolveAbbreviation(std::string_view crypto);
private:
    dpp::task<dpp::command_result> invest(const std::string& crypto, const cash_in& amountIn);
    dpp::task<dpp::command_result> investments(const std::optional<dpp::guild_member_in>& memberOpt);
    dpp::task<dpp::command_result> prices();
    dpp::task<dpp::command_result> withdraw(const std::string& crypto, long double amount);
};
