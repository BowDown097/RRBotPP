#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class General : public dppcmd::module<General>
{
public:
    General();
private:
    dppcmd::command_result achievements(const std::optional<dpp::guild_member>& memberOpt);
    dppcmd::command_result help(const std::optional<std::string>& commandName);
    dppcmd::command_result info();
    dppcmd::command_result module(const std::string& moduleName);
    dppcmd::command_result modules();
    dpp::task<dppcmd::command_result> serverInfo();
    dppcmd::command_result stats(const std::optional<dpp::guild_member>& memberOpt);
    dppcmd::command_result userInfo(std::optional<dpp::guild_member> memberOpt);
};
