#ifndef ADMINISTRATION_H
#define ADMINISTRATION_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Administration : public ModuleBase
{
public:
    Administration();
    MODULE_SETUP(Administration)
private:
    dpp::task<CommandResult> clearTextChannel(const ChannelTypeReader& channelRead);
    CommandResult drawPot();
    CommandResult removeAchievement(const UserTypeReader& userRead, const std::string& name);
    CommandResult removeCrates(const UserTypeReader& userRead);
    CommandResult removeStat(const UserTypeReader& userRead, const std::string& stat);
    CommandResult resetCooldowns(const UserTypeReader& userRead);
    dpp::task<CommandResult> setCash(const UserTypeReader& userRead, long double amount);
    CommandResult setCrypto(const UserTypeReader& userRead, const std::string& crypto, long double amount);
    CommandResult setPrestige(const UserTypeReader& userRead, int level);
    CommandResult setStat(const UserTypeReader& userRead, const std::string& stat, const std::string& value);
    CommandResult unlockAchievement(const UserTypeReader& userRead, const std::string& name);
};

#endif // ADMINISTRATION_H
