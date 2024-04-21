#ifndef GENERAL_H
#define GENERAL_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class General : public ModuleBase
{
public:
    General();
    MODULE_SETUP(General)
private:
    CommandResult achievements(const std::optional<UserTypeReader>& userOpt);
    CommandResult help(const std::optional<std::string>& commandName);
    CommandResult info();
    CommandResult module(const std::string& moduleName);
    CommandResult modules();
    dpp::task<CommandResult> serverInfo();
    CommandResult stats(const std::optional<UserTypeReader>& userOpt);
    CommandResult userInfo(const std::optional<UserTypeReader>& userOpt);
};

#endif // GENERAL_H
