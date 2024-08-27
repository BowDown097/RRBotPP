#pragma once
#include "dppcmd/modules/module.h"
#include "dppinteract/interactiveresult.h"

namespace dpp { class guild_member; class message; }

class DbUser;

class Crime : public dppcmd::module<Crime>
{
public:
    Crime();
private:
    dpp::task<dppcmd::command_result> bully(dpp::guild_member member, const dppcmd::remainder<std::string>& nickname);
    dpp::task<dppcmd::command_result> deal();
    dpp::task<dppcmd::command_result> hack(const dpp::guild_member& member, const std::string& crypto, long double amount);
    dpp::task<dppcmd::command_result> loot();
    dpp::task<dppcmd::command_result> rape(const dpp::guild_member& member);
    dpp::task<dppcmd::command_result> rob(const dpp::guild_member& member, long double amount);
    dpp::task<dppcmd::command_result> scavenge();
    dpp::task<dppcmd::command_result> slavery();
    dpp::task<dppcmd::command_result> whore();

    dpp::task<dppcmd::command_result> genericCrime(const std::span<const std::string_view>& successOutcomes,
                                                   const std::span<const std::string_view>& failOutcomes,
                                                   DbUser& user, int64_t& cooldown, bool hasMehOutcome = false);
    dpp::task<void> handleScavenge(dpp::message& msg, const dppinteract::interactive_result<dpp::message>& result,
                                   DbUser& user, const dpp::guild_member& member,
                                   bool successCondition, std::string_view successResponse,
                                   std::string_view timeoutResponse, std::string_view failureResponse);
    void statUpdate(DbUser& user, bool success, long double gain);
};
