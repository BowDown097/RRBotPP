#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class channel; class guild_member; }

class Administration : public dpp::module<Administration>
{
public:
    Administration();
private:
    dpp::task<dpp::command_result> clearTextChannel(dpp::channel* channel);
    dpp::command_result drawPot();
    dpp::command_result giveItem(const dpp::guild_member& member, const dpp::remainder<std::string>& itemIn);
    dpp::command_result removeAchievement(const dpp::guild_member& member, const dpp::remainder<std::string>& name);
    dpp::command_result removeCrates(const dpp::guild_member& member);
    dpp::command_result removeStat(const dpp::guild_member& member, const dpp::remainder<std::string>& stat);
    dpp::command_result resetCooldowns(const dpp::guild_member& member);
    dpp::task<dpp::command_result> setCash(const dpp::guild_member& member, long double amount);
    dpp::command_result setCrypto(const dpp::guild_member& member, const std::string& crypto, long double amount);
    dpp::command_result setPrestige(const dpp::guild_member& member, int level);
    dpp::command_result setStat(const dpp::guild_member& member, const std::string& stat,
                                const dpp::remainder<std::string>& value);
    dpp::command_result unlockAchievement(const dpp::guild_member& member, const dpp::remainder<std::string>& name);
};
