#ifndef BOTOWNER_H
#define BOTOWNER_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class BotOwner : public ModuleBase
{
public:
    BotOwner();
    MODULE_SETUP(BotOwner)
private:
    CommandResult blacklist(const UserTypeReader& userRead);
    CommandResult disableCommandGlobal(const std::string& cmd);
    CommandResult enableCommandGlobal(const std::string& cmd);
    CommandResult resetUser(const UserTypeReader& userRead);
    CommandResult unblacklist(const UserTypeReader& userRead);
};

#endif // BOTOWNER_H
