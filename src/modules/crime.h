#ifndef CRIME_H
#define CRIME_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/guildmembertypereader.h"
#include "readers/cashtypereader.h"

class Crime : public dpp::module_base
{
public:
    Crime();
    MODULE_SETUP(Crime)
private:
    dpp::task<dpp::command_result> bully(const dpp::guild_member_in& memberIn, const dpp::remainder<std::string>& nickname);
    dpp::task<dpp::command_result> deal();
    dpp::task<dpp::command_result> loot();
    dpp::task<dpp::command_result> rape(const dpp::guild_member_in& memberIn);
    dpp::task<dpp::command_result> rob(const dpp::guild_member_in& memberIn, const cash_in& amountIn);
    dpp::task<dpp::command_result> slavery();
    dpp::task<dpp::command_result> whore();

    dpp::task<dpp::command_result> genericCrime(const std::span<const std::string_view>& successOutcomes,
                                                const std::span<const std::string_view>& failOutcomes,
                                                class DbUser& user, int64_t& cooldown, bool hasMehOutcome = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};

#endif // CRIME_H
