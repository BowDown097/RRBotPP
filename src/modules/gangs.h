#pragma once
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"
#include "readers/cashtypereader.h"

class Gangs : public dpp::module_base
{
public:
    Gangs();
    MODULE_SETUP(Gangs)
private:
    dpp::task<dpp::command_result> buyVault();
    dpp::task<dpp::command_result> createGang(const dpp::remainder<std::string>& name);
    dpp::task<dpp::command_result> deposit(const cash_in& amountIn);
    dpp::command_result disband();
    dpp::command_result gang(const std::optional<dpp::remainder<std::string>>& nameIn);
    dpp::command_result invite(const dpp::user_in& userIn);
    dpp::command_result joinGang(const dpp::remainder<std::string>& name);
    dpp::command_result kickGangMember(const dpp::user_in& userIn);
    dpp::command_result leaveGang();
    dpp::task<dpp::command_result> renameGang(const dpp::remainder<std::string>& name);
    dpp::command_result setPosition(const dpp::user_in& userIn, const dpp::remainder<std::string>& position);
    dpp::command_result togglePublic();
    dpp::command_result transferLeadership(const dpp::user_in& userIn);
    dpp::command_result vaultBalance();
    dpp::task<dpp::command_result> withdrawVault(const cash_in& amountIn);
};
