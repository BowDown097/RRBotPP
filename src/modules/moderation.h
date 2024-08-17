#pragma once
#include "dpp-command-handler/modules/module.h"
#include "readers/rrguildmembertypereader.h"

namespace dpp { class message; }

class Moderation : public dpp::module<Moderation>
{
public:
    Moderation();
private:
    dpp::task<dpp::command_result> ban(const RR::guild_member_in& memberIn,
                                       const std::optional<std::string>& duration,
                                       const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> chill(const std::string& duration);
    dpp::task<dpp::command_result> hackban(uint64_t userId, const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> kick(const RR::guild_member_in& memberIn,
                                        const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> mute(const RR::guild_member_in& memberIn, const std::string& duration,
                                        const std::optional<dpp::remainder<std::string>>& reason);
    dpp::task<dpp::command_result> purge(int limit);
    dpp::task<dpp::command_result> purgeRange(uint64_t from, uint64_t to);
    dpp::task<dpp::command_result> purgeUser(const RR::guild_member_in& memberIn, const std::optional<int>& limitIn);
    dpp::task<dpp::command_result> unban(uint64_t userId);
    dpp::task<dpp::command_result> unchill();
    dpp::task<dpp::command_result> unmute(const RR::guild_member_in& memberIn);
};
