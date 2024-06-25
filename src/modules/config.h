#pragma once
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "dpp-command-handler/readers/roletypereader.h"
#include "readers/cashtypereader.h"

class Config : public dpp::module<Config>
{
public:
    Config();
private:
    dpp::command_result addRank(int level, const cash_in& costIn, const dpp::role_in& roleIn);
    dpp::command_result clearConfig();
    dpp::command_result currentConfig();
    dpp::command_result disableCommand(const std::string& cmd);
    dpp::command_result disableFiltersInChannel(const dpp::channel_in& channelIn);
    dpp::command_result disableModule(const std::string& module);
    dpp::command_result enableCommand(const std::string& cmd);
    dpp::command_result enableModule(const std::string& module);
    dpp::command_result filterTerm(const dpp::remainder<std::string>& term);
    dpp::command_result setAdminRole(const dpp::role_in& roleIn);
    dpp::command_result setDjRole(const dpp::role_in& roleIn);
    dpp::command_result setLogsChannel(const dpp::channel_in& channelIn);
    dpp::command_result setModRole(const dpp::role_in& roleIn);
    dpp::command_result setPotChannel(const dpp::channel_in& channelIn);
    dpp::command_result toggleDrops();
    dpp::command_result toggleInviteFilter();
    dpp::command_result toggleNsfw();
    dpp::command_result toggleScamFilter();
    dpp::command_result unfilterTerm(const dpp::remainder<std::string>& term);
    dpp::command_result unwhitelistChannel(const dpp::channel_in& channelIn);
    dpp::command_result whitelistChannel(const dpp::channel_in& channelIn);
};
