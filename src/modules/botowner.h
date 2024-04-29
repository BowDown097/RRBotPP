#ifndef BOTOWNER_H
#define BOTOWNER_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class BotOwner : public dpp::module_base
{
public:
    BotOwner();
    MODULE_SETUP(BotOwner)
private:
    dpp::command_result blacklist(const dpp::user_in& userIn);
    dpp::command_result disableCommandGlobal(const std::string& cmd);
    dpp::command_result enableCommandGlobal(const std::string& cmd);
    dpp::command_result resetUser(const dpp::user_in& userIn);
    dpp::command_result unblacklist(const dpp::user_in& userIn);
};

#endif // BOTOWNER_H
