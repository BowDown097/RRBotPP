#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class Gangs : public dpp::module<Gangs>
{
public:
    Gangs();
private:
    dpp::task<dpp::command_result> buyVault();
    dpp::task<dpp::command_result> createGang(const dpp::remainder<std::string>& name);
    dpp::task<dpp::command_result> deposit(long double amount);
    dpp::command_result disband();
    dpp::command_result gang(const std::optional<dpp::remainder<std::string>>& nameIn);
    dpp::command_result invite(const dpp::guild_member& member);
    dpp::command_result joinGang(const dpp::remainder<std::string>& name);
    dpp::command_result kickGangMember(const dpp::guild_member& member);
    dpp::command_result leaveGang();
    dpp::task<dpp::command_result> renameGang(const dpp::remainder<std::string>& name);
    dpp::command_result setPosition(const dpp::guild_member& member, const dpp::remainder<std::string>& position);
    dpp::command_result togglePublic();
    dpp::command_result transferLeadership(const dpp::guild_member& member);
    dpp::command_result vaultBalance();
    dpp::task<dpp::command_result> withdrawVault(long double amount);
};
