#pragma once
#include "dpp-command-handler/modules/module.h"
#include "readers/rrguildmembertypereader.h"

class General : public dpp::module<General>
{
public:
    General();
private:
    dpp::command_result achievements(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::command_result help(const std::optional<std::string>& commandName);
    dpp::command_result info();
    dpp::command_result module(const std::string& moduleName);
    dpp::command_result modules();
    dpp::task<dpp::command_result> serverInfo();
    dpp::command_result stats(const std::optional<RR::guild_member_in>& memberOpt);
    dpp::command_result userInfo(const std::optional<RR::guild_member_in>& memberOpt);
};
