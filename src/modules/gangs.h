#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Gangs : public dppcmd::module<Gangs>
{
public:
    Gangs();
private:
    dpp::task<dppcmd::command_result> buyVault();
    dpp::task<dppcmd::command_result> createGang(const dppcmd::remainder<std::string>& name);
    dpp::task<dppcmd::command_result> deposit(long double amount);
    dppcmd::command_result disband();
    dppcmd::command_result gang(const std::optional<dppcmd::remainder<std::string>>& nameIn);
    dppcmd::command_result invite(const dpp::guild_member& member);
    dppcmd::command_result joinGang(const dppcmd::remainder<std::string>& name);
    dppcmd::command_result kickGangMember(const dpp::guild_member& member);
    dppcmd::command_result leaveGang();
    dpp::task<dppcmd::command_result> renameGang(const dppcmd::remainder<std::string>& name);
    dppcmd::command_result setPosition(const dpp::guild_member& member, const dppcmd::remainder<std::string>& position);
    dppcmd::command_result togglePublic();
    dppcmd::command_result transferLeadership(const dpp::guild_member& member);
    dppcmd::command_result vaultBalance();
    dpp::task<dppcmd::command_result> withdrawVault(long double amount);
};
