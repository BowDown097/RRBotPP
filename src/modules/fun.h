#pragma once
#include "dpp-command-handler/modules/module.h"
#include "readers/rrguildmembertypereader.h"

class Fun : public dpp::module<Fun>
{
public:
    Fun();
private:
    dpp::task<dpp::command_result> cat();
    dpp::task<dpp::command_result> dog();
    dpp::command_result flip();
    dpp::command_result gay(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::task<dpp::command_result> godword(const std::optional<int>& amountOpt);
    dpp::command_result magicConch(const dpp::remainder<std::string>&);
    dpp::command_result penis(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::command_result sneed();
    dpp::task<dpp::command_result> terryQuote();
    dpp::command_result waifu();
};
