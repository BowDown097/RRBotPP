#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class channel; class guild_member; }

class Administration : public dppcmd::module<Administration>
{
public:
    Administration();
private:
    dpp::task<dppcmd::command_result> clearTextChannel(dpp::channel* channel);
    dppcmd::command_result drawPot();
    dppcmd::command_result giveItem(const dpp::guild_member& member, const dppcmd::remainder<std::string>& itemIn);
    dppcmd::command_result removeAchievement(const dpp::guild_member& member, const dppcmd::remainder<std::string>& name);
    dppcmd::command_result removeCrates(const dpp::guild_member& member);
    dppcmd::command_result removeStat(const dpp::guild_member& member, const dppcmd::remainder<std::string>& stat);
    dppcmd::command_result resetCooldowns(const dpp::guild_member& member);
    dpp::task<dppcmd::command_result> setCash(const dpp::guild_member& member, long double amount);
    dppcmd::command_result setCrypto(const dpp::guild_member& member, const std::string& crypto, long double amount);
    dppcmd::command_result setPrestige(const dpp::guild_member& member, int level);
    dppcmd::command_result setStat(const dpp::guild_member& member, const std::string& stat,
                                   const dppcmd::remainder<std::string>& value);
    dppcmd::command_result unlockAchievement(const dpp::guild_member& member, const dppcmd::remainder<std::string>& name);
};
