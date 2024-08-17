#pragma once
#include "dpp-command-handler/modules/module.h"
#include "readers/rrguildmembertypereader.h"

class BotOwner : public dpp::module<BotOwner>
{
public:
    BotOwner();
private:
    dpp::command_result blacklist(const RR::guild_member_in& memberIn);
    dpp::command_result disableCommandGlobal(const std::string& cmd);
    dpp::command_result enableCommandGlobal(const std::string& cmd);
    dpp::command_result resetUser(const RR::guild_member_in& memberIn);
    dpp::command_result unblacklist(const RR::guild_member_in& memberIn);
};
