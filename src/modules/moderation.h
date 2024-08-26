#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; class message; }

class Moderation : public dppcmd::module<Moderation>
{
public:
    Moderation();
private:
    dpp::task<dppcmd::command_result> ban(const dpp::guild_member& member,
                                          const std::optional<std::string>& duration,
                                          const std::optional<dppcmd::remainder<std::string>>& reason);
    dpp::task<dppcmd::command_result> chill(const std::string& duration);
    dpp::task<dppcmd::command_result> hackban(uint64_t userId, const std::optional<dppcmd::remainder<std::string>>& reason);
    dpp::task<dppcmd::command_result> kick(const dpp::guild_member& member,
                                           const std::optional<dppcmd::remainder<std::string>>& reason);
    dpp::task<dppcmd::command_result> mute(const dpp::guild_member& member, const std::string& duration,
                                           const std::optional<dppcmd::remainder<std::string>>& reason);
    dpp::task<dppcmd::command_result> purge(int limit);
    dpp::task<dppcmd::command_result> purgeRange(uint64_t from, uint64_t to);
    dpp::task<dppcmd::command_result> purgeUser(const dpp::guild_member& member, const std::optional<int>& limitIn);
    dpp::task<dppcmd::command_result> unban(uint64_t userId);
    dpp::task<dppcmd::command_result> unchill();
    dpp::task<dppcmd::command_result> unmute(const dpp::guild_member& member);
};
