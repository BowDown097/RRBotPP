#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class BotOwner : public dppcmd::module<BotOwner>
{
public:
    BotOwner();
private:
    dppcmd::command_result blacklist(const dpp::guild_member& member);
    dppcmd::command_result disableCommandGlobal(const std::string& cmd);
    dppcmd::command_result enableCommandGlobal(const std::string& cmd);
    dppcmd::command_result resetUser(const dpp::guild_member& member);
    dppcmd::command_result unblacklist(const dpp::guild_member& member);
};
