#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class General : public dpp::module<General>
{
public:
    General();
private:
    dpp::command_result achievements(const std::optional<dpp::guild_member>& memberOpt);
    dpp::command_result help(const std::optional<std::string>& commandName);
    dpp::command_result info();
    dpp::command_result module(const std::string& moduleName);
    dpp::command_result modules();
    dpp::task<dpp::command_result> serverInfo();
    dpp::command_result stats(const std::optional<dpp::guild_member>& memberOpt);
    dpp::command_result userInfo(std::optional<dpp::guild_member> memberOpt);
};
