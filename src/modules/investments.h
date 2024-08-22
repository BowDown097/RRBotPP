#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class Investments : public dpp::module<Investments>
{
public:
    Investments();
    static dpp::task<std::optional<long double>> queryCryptoValue(std::string_view crypto, dpp::cluster* cluster);
    static std::string resolveAbbreviation(std::string_view crypto);
private:
    dpp::task<dpp::command_result> invest(const std::string& crypto, long double cashAmount);
    dpp::task<dpp::command_result> investments(std::optional<dpp::guild_member> memberOpt);
    dpp::task<dpp::command_result> prices();
    dpp::task<dpp::command_result> withdraw(const std::string& crypto, long double cryptoAmount);
};
