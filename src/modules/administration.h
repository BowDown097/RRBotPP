#pragma once
#include "dpp-command-handler/modules/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "dpp-command-handler/readers/guildmembertypereader.h"
#include "dpp-command-handler/readers/usertypereader.h"
#include "readers/cashtypereader.h"

class Administration : public dpp::module<Administration>
{
public:
    Administration();
private:
    dpp::task<dpp::command_result> clearTextChannel(const dpp::channel_in& channelIn);
    dpp::command_result drawPot();
    dpp::command_result giveItem(const dpp::user_in& userIn, const dpp::remainder<std::string>& itemIn);
    dpp::command_result removeAchievement(const dpp::user_in& userIn, const dpp::remainder<std::string>& name);
    dpp::command_result removeCrates(const dpp::user_in& userIn);
    dpp::command_result removeStat(const dpp::user_in& userIn, const dpp::remainder<std::string>& stat);
    dpp::command_result resetCooldowns(const dpp::user_in& userIn);
    dpp::task<dpp::command_result> setCash(const dpp::guild_member_in& memberIn, const cash_in& amountIn);
    dpp::command_result setCrypto(const dpp::user_in& userIn, const std::string& crypto, long double amount);
    dpp::command_result setPrestige(const dpp::user_in& userIn, int level);
    dpp::command_result setStat(const dpp::user_in& userIn, const std::string& stat,
                                const dpp::remainder<std::string>& value);
    dpp::command_result unlockAchievement(const dpp::user_in& userIn, const dpp::remainder<std::string>& name);
};
