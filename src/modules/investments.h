#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Investments : public dppcmd::module<Investments>
{
public:
    Investments();
    static dpp::task<std::optional<long double>> queryCryptoValue(std::string_view crypto, dpp::cluster* cluster);
    static std::string resolveAbbreviation(std::string_view crypto);
private:
    dpp::task<dppcmd::command_result> invest(const std::string& crypto, long double cashAmount);
    dpp::task<dppcmd::command_result> investments(std::optional<dpp::guild_member> memberOpt);
    dpp::task<dppcmd::command_result> prices();
    dpp::task<dppcmd::command_result> withdraw(const std::string& crypto, long double cryptoAmount);
};
