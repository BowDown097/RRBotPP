#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Fun : public dppcmd::module<Fun>
{
public:
    Fun();
private:
    dpp::task<dppcmd::command_result> cat();
    dpp::task<dppcmd::command_result> dog();
    dppcmd::command_result flip();
    dppcmd::command_result gay(const std::optional<dpp::guild_member>& memberOpt);
    dpp::task<dppcmd::command_result> godword(const std::optional<int>& amountOpt);
    dppcmd::command_result magicConch(const dppcmd::remainder<std::string>&);
    dppcmd::command_result penis(const std::optional<dpp::guild_member>& memberOpt);
    dppcmd::command_result sneed();
    dpp::task<dppcmd::command_result> terryQuote();
    dppcmd::command_result waifu();
};
