#pragma once
#include "dpp-command-handler/modules/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class BotOwner : public dpp::module<BotOwner>
{
public:
    BotOwner();
private:
    dpp::command_result blacklist(const dpp::user_in& userIn);
    dpp::command_result disableCommandGlobal(const std::string& cmd);
    dpp::command_result enableCommandGlobal(const std::string& cmd);
    dpp::command_result resetUser(const dpp::user_in& userIn);
    dpp::command_result unblacklist(const dpp::user_in& userIn);
};
