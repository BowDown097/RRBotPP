#pragma once
#include "dpp-command-handler/modules/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "readers/cashtypereader.h"
#include "readers/rrguildmembertypereader.h"

class Administration : public dpp::module<Administration>
{
public:
    Administration();
private:
    dpp::task<dpp::command_result> clearTextChannel(const dpp::channel_in& channelIn);
    dpp::command_result drawPot();
    dpp::command_result giveItem(const RR::guild_member_in& memberIn, const dpp::remainder<std::string>& itemIn);
    dpp::command_result removeAchievement(const RR::guild_member_in& memberIn, const dpp::remainder<std::string>& name);
    dpp::command_result removeCrates(const RR::guild_member_in& memberIn);
    dpp::command_result removeStat(const RR::guild_member_in& memberIn, const dpp::remainder<std::string>& stat);
    dpp::command_result resetCooldowns(const RR::guild_member_in& memberIn);
    dpp::task<dpp::command_result> setCash(const RR::guild_member_in& memberIn, const cash_in& amountIn);
    dpp::command_result setCrypto(const RR::guild_member_in& memberIn, const std::string& crypto, long double amount);
    dpp::command_result setPrestige(const RR::guild_member_in& memberIn, int level);
    dpp::command_result setStat(const RR::guild_member_in& memberIn, const std::string& stat,
                                const dpp::remainder<std::string>& value);
    dpp::command_result unlockAchievement(const RR::guild_member_in& memberIn, const dpp::remainder<std::string>& name);
};
