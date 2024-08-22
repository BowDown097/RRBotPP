#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class channel; class role; }

class Config : public dpp::module<Config>
{
public:
    Config();
private:
    dpp::command_result addRank(int level, long double cost, dpp::role* role);
    dpp::command_result clearConfig();
    dpp::command_result currentConfig();
    dpp::command_result disableCommand(const std::string& cmd);
    dpp::command_result disableFiltersInChannel(dpp::channel* channel);
    dpp::command_result disableModule(const std::string& module);
    dpp::command_result enableCommand(const std::string& cmd);
    dpp::command_result enableModule(const std::string& module);
    dpp::command_result filterTerm(const dpp::remainder<std::string>& term);
    dpp::command_result setAdminRole(dpp::role* role);
    dpp::command_result setDjRole(dpp::role* role);
    dpp::command_result setLogsChannel(dpp::channel* channel);
    dpp::command_result setModRole(dpp::role* role);
    dpp::command_result setPotChannel(dpp::channel* channel);
    dpp::command_result toggleDrops();
    dpp::command_result toggleInviteFilter();
    dpp::command_result toggleNsfw();
    dpp::command_result toggleScamFilter();
    dpp::command_result unfilterTerm(const dpp::remainder<std::string>& term);
    dpp::command_result unwhitelistChannel(dpp::channel* channel);
    dpp::command_result whitelistChannel(dpp::channel* channel);
};
