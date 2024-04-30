#ifndef GENERAL_H
#define GENERAL_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/guildmembertypereader.h"
#include "dpp-command-handler/readers/usertypereader.h"

class General : public dpp::module_base
{
public:
    General();
    MODULE_SETUP(General)
private:
    dpp::command_result achievements(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result help(const std::optional<std::string>& commandName);
    dpp::command_result info();
    dpp::command_result module(const std::string& moduleName);
    dpp::command_result modules();
    dpp::task<dpp::command_result> serverInfo();
    dpp::command_result stats(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result userInfo(const std::optional<dpp::guild_member_in>& memberOpt);
};

#endif // GENERAL_H
