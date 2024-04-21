#ifndef ECONOMY_H
#define ECONOMY_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Economy : public ModuleBase
{
public:
    Economy();
    MODULE_SETUP(Economy)
private:
    CommandResult balance(const std::optional<UserTypeReader>& userOpt);
    CommandResult cooldowns(const std::optional<UserTypeReader>& userOpt);
    CommandResult profile(const std::optional<UserTypeReader>& userOpt);
    CommandResult ranks();
    dpp::task<CommandResult> sauce(const UserTypeReader& userRead, long double amount);
};

#endif // ECONOMY_H
