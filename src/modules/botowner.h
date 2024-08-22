#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class BotOwner : public dpp::module<BotOwner>
{
public:
    BotOwner();
private:
    dpp::command_result blacklist(const dpp::guild_member& member);
    dpp::command_result disableCommandGlobal(const std::string& cmd);
    dpp::command_result enableCommandGlobal(const std::string& cmd);
    dpp::command_result resetUser(const dpp::guild_member& member);
    dpp::command_result unblacklist(const dpp::guild_member& member);
};
