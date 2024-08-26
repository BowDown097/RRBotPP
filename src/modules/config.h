#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class channel; class role; }

class Config : public dppcmd::module<Config>
{
public:
    Config();
private:
    dppcmd::command_result addRank(int level, long double cost, dpp::role* role);
    dppcmd::command_result clearConfig();
    dppcmd::command_result currentConfig();
    dppcmd::command_result disableCommand(const std::string& cmd);
    dppcmd::command_result disableFiltersInChannel(dpp::channel* channel);
    dppcmd::command_result disableModule(const std::string& module);
    dppcmd::command_result enableCommand(const std::string& cmd);
    dppcmd::command_result enableModule(const std::string& module);
    dppcmd::command_result filterTerm(const dppcmd::remainder<std::string>& term);
    dppcmd::command_result setAdminRole(dpp::role* role);
    dppcmd::command_result setDjRole(dpp::role* role);
    dppcmd::command_result setLogsChannel(dpp::channel* channel);
    dppcmd::command_result setModRole(dpp::role* role);
    dppcmd::command_result setPotChannel(dpp::channel* channel);
    dppcmd::command_result toggleDrops();
    dppcmd::command_result toggleInviteFilter();
    dppcmd::command_result toggleNsfw();
    dppcmd::command_result toggleScamFilter();
    dppcmd::command_result unfilterTerm(const dppcmd::remainder<std::string>& term);
    dppcmd::command_result unwhitelistChannel(dpp::channel* channel);
    dppcmd::command_result whitelistChannel(dpp::channel* channel);
};
