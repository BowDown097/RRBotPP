#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; class message; }

class Moderation : public dpp::module<Moderation>
{
public:
    Moderation();
private:
    dpp::task<dpp::command_result> ban(const dpp::guild_member& member,
                                       const std::optional<std::string>& duration,
                                       const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> chill(const std::string& duration);
    dpp::task<dpp::command_result> hackban(uint64_t userId, const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> kick(const dpp::guild_member& member,
                                        const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> mute(const dpp::guild_member& member, const std::string& duration,
                                        const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> purge(int limit);
    dpp::task<dpp::command_result> purgeRange(uint64_t from, uint64_t to);
    dpp::task<dpp::command_result> purgeUser(const dpp::guild_member& member, const std::optional<int>& limitIn);
    dpp::task<dpp::command_result> unban(uint64_t userId);
    dpp::task<dpp::command_result> unchill();
    dpp::task<dpp::command_result> unmute(const dpp::guild_member& member);
};
