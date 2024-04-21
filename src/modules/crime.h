#ifndef CRIME_H
#define CRIME_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Crime : public ModuleBase
{
public:
    Crime();
    MODULE_SETUP(Crime)
private:
    dpp::task<CommandResult> bully(const UserTypeReader& userRead, const std::string& nickname);
    dpp::task<CommandResult> deal();
    dpp::task<CommandResult> hack(const UserTypeReader& userRead, const std::string& crypto, long double amount);
    dpp::task<CommandResult> loot();
    dpp::task<CommandResult> rape(const UserTypeReader& userRead);
    dpp::task<CommandResult> rob(const UserTypeReader& userRead, long double amount);
    dpp::task<CommandResult> scavenge();
    dpp::task<CommandResult> slavery();
    dpp::task<CommandResult> whore();

    dpp::task<CommandResult> genericCrime(const std::span<const std::string_view>& successOutcomes,
                                          const std::span<const std::string_view>& failOutcomes,
                                          class DbUser& user, int64_t& cooldown, bool hasMehOutcome = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};

#endif // CRIME_H
