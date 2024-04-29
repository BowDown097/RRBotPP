#ifndef ADMINISTRATION_H
#define ADMINISTRATION_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "dpp-command-handler/readers/usertypereader.h"
#include "readers/cashtypereader.h"

class Administration : public dpp::module_base
{
public:
    Administration();
    MODULE_SETUP(Administration)
private:
    dpp::task<dpp::command_result> clearTextChannel(const dpp::channel_in& channelIn);
    dpp::command_result drawPot();
    dpp::command_result removeAchievement(const dpp::user_in& userIn, const std::string& name);
    dpp::command_result removeCrates(const dpp::user_in& userIn);
    dpp::command_result removeStat(const dpp::user_in& userIn, const std::string& stat);
    dpp::command_result resetCooldowns(const dpp::user_in& userIn);
    dpp::task<dpp::command_result> setCash(const dpp::user_in& userIn, const cash_in& amountIn);
    dpp::command_result setCrypto(const dpp::user_in& userIn, const std::string& crypto, long double amount);
    dpp::command_result setPrestige(const dpp::user_in& userIn, int level);
    dpp::command_result setStat(const dpp::user_in& userIn, const std::string& stat, const std::string& value);
    dpp::command_result unlockAchievement(const dpp::user_in& userIn, const std::string& name);
};

#endif // ADMINISTRATION_H
