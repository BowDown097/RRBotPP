#include "moderation.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigroles.h"
#include "database/entities/dbban.h"
#include "database/entities/dbchill.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "utils/dpp.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>
#include <dpp/guild.h>

Moderation::Moderation() : dppcmd::module<Moderation>("Moderation", "This is like George Orwell's, uh book, 1984.")
{
    register_command(&Moderation::ban, std::in_place, { "ban", "seethe" }, "Ban any member.", "$ban [user] <duration> <reason>");
    register_command(&Moderation::chill, std::in_place, "chill", "Shut chat up for a specific amount of time.", "$chill [duration]");
    register_command(&Moderation::hackban, std::in_place, "hackban", "Ban any member, even if they are not in the server.", "$hackban [userId] <reason>");
    register_command(&Moderation::kick, std::in_place, { "kick", "cope" }, "Kick any member.", "$kick [user] <reason>");
    register_command(&Moderation::mute, std::in_place, { "mute", "1984", "timeout" }, "Mute any member for any amount of time for any reason.", "$mute [user] [duration] <reason>");
    register_command(&Moderation::purge, std::in_place, { "purge", "clear" }, "Delete any amount of messages (Note: messages that are 2+ weeks old will not be caught by this command).", "$purge [limit]");
    register_command(&Moderation::purgeRange, std::in_place, { "purgerange", "clearrange" }, "Delete a range of message IDs (Note: messages that are 2+ weeks old will not be caught by this command).", "$purgerange [from] [to]");
    register_command(&Moderation::purgeUser, std::in_place, { "purgeuser", "clearuser" }, "Delete any amount of messages from a user (Note: messages that are 2+ weeks old will not be caught by this command).", "$purgeuser [user] <limit>");
    register_command(&Moderation::unban, std::in_place, "unban", "Unban any currently banned member.", "$unban [userId]");
    register_command(&Moderation::unchill, std::in_place, { "unchill", "thaw" }, "Let chat talk again.");
    register_command(&Moderation::unmute, std::in_place, { "unmute", "1985", "untimeout" }, "Unmute any member.", "$unmute [user]");
}

dpp::task<dppcmd::command_result> Moderation::ban(const dpp::guild_member& member,
                                                  const std::optional<std::string>& duration,
                                                  const std::optional<dppcmd::remainder<std::string>>& reason)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbConfigRoles roles = MongoManager::fetchRoleConfig(member.guild_id);
    if (roles.memberIsStaff(member))
        co_return dppcmd::command_result::from_error(std::format(Responses::CantDoActionOnStaff, "ban", member.get_mention()));

    DbUser dbUser = MongoManager::fetchUser(member.user_id, member.guild_id);
    std::string response;
    if (duration.has_value())
    {
        if (long time = RR::utility::resolveDurationString(duration.value()))
        {
            response = std::format(Responses::BannedUser, member.get_mention()) + " for " + RR::utility::formatSeconds(time);
            if (reason.has_value())
                response = " for \"" + *reason.value() + "\"";
            response += '.';

            DbBan dbBan = MongoManager::fetchBan(member.user_id, member.guild_id);
            dbBan.time = time;
            MongoManager::updateBan(dbBan);

            dpp::confirmation_callback_t conf = co_await cluster->set_audit_reason(*reason.value()).co_guild_ban_add(member.guild_id, member.user_id);
            if (conf.is_error())
                co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Ban", conf.get_error().human_readable));
        }
        else
        {
            co_return dppcmd::command_result::from_error(Responses::InvalidDuration);
        }
    }
    else
    {
        dpp::confirmation_callback_t conf = co_await cluster->set_audit_reason(*reason.value()).co_guild_ban_add(member.guild_id, member.user_id);
        if (conf.is_error())
            co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Ban", conf.get_error().human_readable));
        response = std::format(Responses::BannedUser, member.get_mention()) + '.';
    }

    dbUser.mergeStat("Bans", "1");
    MongoManager::updateUser(dbUser);
    co_return dppcmd::command_result::from_success(response);
}

dpp::task<dppcmd::command_result> Moderation::chill(const std::string& duration)
{
    if (long time = RR::utility::resolveDurationString(duration))
    {
        if (time < Constants::ChillMinSeconds)
            co_return dppcmd::command_result::from_error(std::format(Responses::InvalidChillDuration, "less", RR::utility::formatSeconds(Constants::ChillMinSeconds)));
        if (time > Constants::ChillMaxSeconds)
            co_return dppcmd::command_result::from_error(std::format(Responses::InvalidChillDuration, "more", RR::utility::formatSeconds(Constants::ChillMaxSeconds)));

        if (const dpp::channel* channel = dpp::find_channel(context->msg.channel_id))
        {
            std::vector<dpp::permission_overwrite> overwrites = channel->permission_overwrites;
            auto it = std::ranges::find_if(overwrites, [this](const auto& ow) { return ow.id == context->msg.guild_id; });
            if (it == overwrites.end())
                co_return dppcmd::command_result::from_error(Responses::GetEveryoneOverwriteFailed);
            if (it->deny.has(dpp::permissions::p_send_messages))
                co_return dppcmd::command_result::from_error(Responses::ChannelAlreadyChilled);

            DbChill dbChill = MongoManager::fetchChill(context->msg.channel_id, context->msg.guild_id);
            dbChill.time = time;
            MongoManager::updateChill(dbChill);

            it->deny.add(dpp::permissions::p_send_messages);

            dpp::confirmation_callback_t conf = co_await cluster->co_channel_edit_permissions(*channel, it->id, it->allow, it->deny, false);
            if (conf.is_error())
                co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Chill", conf.get_error().human_readable));

            co_return dppcmd::command_result::from_success(std::format(Responses::ChannelChilled, RR::utility::formatSeconds(time)));
        }

        co_return dppcmd::command_result::from_error(Responses::GetChannelFailed);
    }

    co_return dppcmd::command_result::from_error(Responses::InvalidDuration);
}

dpp::task<dppcmd::command_result> Moderation::hackban(uint64_t userId, const std::optional<dppcmd::remainder<std::string>>& reason)
{
    dpp::confirmation_callback_t conf = co_await cluster->co_guild_ban_add(context->msg.guild_id, userId);
    if (conf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Hackban", conf.get_error().human_readable));

    std::string response;
    if (const dpp::user* user = dpp::find_user(userId))
    {
        response = "Hackbanned **" + user->global_name + "**";
        if (reason.has_value())
            response += " for \"" + *reason.value() + "\"";
    }
    else
    {
        response = "Hackbanned the user with that ID";
        if (reason.has_value())
            response += " for \"" + *reason.value() + "\"";
    }

    co_return dppcmd::command_result::from_success(response + '.');
}

dpp::task<dppcmd::command_result> Moderation::kick(const dpp::guild_member& member,
    const std::optional<dppcmd::remainder<std::string>>& reason)
{
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbConfigRoles roles = MongoManager::fetchRoleConfig(member.guild_id);
    if (roles.memberIsStaff(member))
        co_return dppcmd::command_result::from_error(std::format(Responses::CantDoActionOnStaff, "kick", member.get_mention()));

    dpp::confirmation_callback_t conf = co_await cluster->co_guild_member_kick(member.guild_id, member.user_id);
    if (conf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Kick", conf.get_error().human_readable));

    DbUser dbUser = MongoManager::fetchUser(member.user_id, member.guild_id);
    dbUser.mergeStat("Kicks", "1");
    MongoManager::updateUser(dbUser);

    std::string response = std::format(Responses::KickedUser, member.get_mention());
    if (reason.has_value())
        response += " for \"" + *reason.value() + "\"";
    response += '.';

    co_return dppcmd::command_result::from_success(response);
}

dpp::task<dppcmd::command_result> Moderation::mute(const dpp::guild_member& member, const std::string& duration,
                                                   const std::optional<dppcmd::remainder<std::string>>& reason)
{
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (member.is_communication_disabled())
        co_return dppcmd::command_result::from_error(std::format(Responses::UserAlreadyMuted, member.get_mention()));

    const dpp::user* user = member.get_user();
    if (!user)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);
    if (user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbConfigRoles roles = MongoManager::fetchRoleConfig(member.guild_id);
    if (roles.memberIsStaff(member))
        co_return dppcmd::command_result::from_error(std::format(Responses::CantDoActionOnStaff, "mute", member.get_mention()));

    if (long time = RR::utility::resolveDurationString(duration))
    {
        if (time > RR::utility::secondsInDays(28))
            co_return dppcmd::command_result::from_error(Responses::MuteTooLong);

        dpp::confirmation_callback_t conf = co_await cluster->co_guild_member_timeout(member.guild_id, member.user_id, RR::utility::unixTimestamp(time));
        if (conf.is_error())
            co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Mute", conf.get_error().human_readable));

        DbUser dbUser = MongoManager::fetchUser(member.user_id, member.guild_id);
        dbUser.mergeStat("Mutes", "1");
        dbUser.unlockAchievement("Literally 1984", context, user);
        MongoManager::updateUser(dbUser);

        std::string response = std::format(Responses::MutedUser, member.get_mention(), RR::utility::formatSeconds(time));
        if (reason.has_value())
            response = " for \"" + *reason.value() + "\"";
        response += '.';

        co_return dppcmd::command_result::from_success(response);
    }

    co_return dppcmd::command_result::from_error(Responses::InvalidDuration);
}

dpp::task<dppcmd::command_result> Moderation::purge(int limit)
{
    if (limit <= 0)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (limit > 100)
        co_return dppcmd::command_result::from_error(Responses::PurgeTooManyMessages);

    co_await cluster->co_message_delete(context->msg.id, context->msg.channel_id);

    dpp::confirmation_callback_t getConf = co_await cluster->co_messages_get(context->msg.channel_id, 0, 0, 0, limit);
    if (getConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", getConf.get_error().human_readable));

    std::vector<dpp::snowflake> messages = RR::utility::getLast14DaysMessages(getConf.get<dpp::message_map>());
    if (messages.empty())
        co_return dppcmd::command_result::from_error(Responses::PurgeFoundNoMessages);

    dpp::confirmation_callback_t delConf = co_await cluster->co_message_delete_bulk(messages, context->msg.channel_id);
    if (delConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", delConf.get_error().human_readable));

    co_return dppcmd::command_result::from_success(std::format(Responses::PurgeSuccess, messages.size() - 1));
}

dpp::task<dppcmd::command_result> Moderation::purgeRange(uint64_t from, uint64_t to)
{
    dpp::confirmation_callback_t getConf = co_await cluster->co_messages_get(context->msg.channel_id, 0, 0, from, 100);
    if (getConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", getConf.get_error().human_readable));

    dpp::message_map messageUnorderedMap = getConf.get<dpp::message_map>();
    std::vector<std::pair<dpp::snowflake, dpp::message>> messageMap(
        std::make_move_iterator(messageUnorderedMap.begin()),
        std::make_move_iterator(messageUnorderedMap.end()));
    std::ranges::sort(messageMap, {}, [](const std::pair<dpp::snowflake, dpp::message>& p) { return p.first; });

    dpp::confirmation_callback_t get2Conf = co_await cluster->co_messages_get(context->msg.channel_id, 0, messageMap.front().first, 0, 1);
    if (get2Conf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", get2Conf.get_error().human_readable));

    dpp::message_map map2 = get2Conf.get<dpp::message_map>();
    messageMap.insert(messageMap.cbegin(), map2.cbegin(), map2.cend());

    std::vector<dpp::snowflake> messages;
    messages.reserve(messageMap.size());

    bool foundEndMessage{};
    long ts14DaysAgo = RR::utility::unixTimestamp(-RR::utility::secondsInDays(14));

    for (const auto& [id, message] : messageMap)
    {
        if (message.sent <= ts14DaysAgo)
            continue;

        messages.push_back(id);

        if (id == to)
        {
            foundEndMessage = true;
            break;
        }
    }

    if (messages.empty())
        co_return dppcmd::command_result::from_error(Responses::PurgeFoundNoMessages);

    dpp::confirmation_callback_t delConf = co_await cluster->co_message_delete_bulk(messages, context->msg.channel_id);
    if (delConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", delConf.get_error().human_readable));

    std::string response = std::format(Responses::PurgeSuccess, messages.size());
    if (!foundEndMessage)
    {
        response += '\n';
        response += Responses::PurgeRangeDidNotReachEnd;
    }

    co_return dppcmd::command_result::from_success(response);
}

dpp::task<dppcmd::command_result> Moderation::purgeUser(const dpp::guild_member& member, const std::optional<int>& limitIn)
{
    dpp::confirmation_callback_t getConf = co_await cluster->co_messages_get(context->msg.channel_id, 0, 0, 0, 100);
    if (getConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", getConf.get_error().human_readable));

    auto filter = [ts14DaysAgo = RR::utility::unixTimestamp(-RR::utility::secondsInDays(14)), userId = member.user_id](const std::pair<dpp::snowflake, dpp::message>& pair) {
        return pair.second.author.id == userId && pair.second.sent > ts14DaysAgo;
    };

    dpp::message_map messageMap = getConf.get<dpp::message_map>();
    auto messagesView = messageMap | std::views::filter(filter) | std::views::keys;
    std::vector<dpp::snowflake> messages = limitIn.has_value()
        ? messagesView | std::views::take(limitIn.value()) | std::ranges::to<std::vector>()
        : std::ranges::to<std::vector>(messagesView);

    if (messages.empty())
        co_return dppcmd::command_result::from_error(Responses::PurgeFoundNoMessages);

    dpp::confirmation_callback_t delConf = co_await cluster->co_message_delete_bulk(messages, context->msg.channel_id);
    if (delConf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Purge", delConf.get_error().human_readable));

    co_return dppcmd::command_result::from_success(std::format(Responses::PurgeSuccess, messages.size()));
}

dpp::task<dppcmd::command_result> Moderation::unban(uint64_t userId)
{
    dpp::confirmation_callback_t conf = co_await cluster->co_guild_ban_delete(context->msg.guild_id, userId);
    if (conf.is_error())
        co_return dppcmd::command_result::from_error(Responses::UnbanFailed);

    MongoManager::deleteBan(userId, context->msg.guild_id);
    if (const dpp::user* user = dpp::find_user(userId))
        co_return dppcmd::command_result::from_success("Unbanned " + user->get_mention() + '.');
    else
        co_return dppcmd::command_result::from_success("Unbanned the user with that ID.");
}

dpp::task<dppcmd::command_result> Moderation::unchill()
{
    if (const dpp::channel* channel = dpp::find_channel(context->msg.channel_id))
    {
        std::vector<dpp::permission_overwrite> overwrites = channel->permission_overwrites;
        auto it = std::ranges::find_if(overwrites, [this](const dpp::permission_overwrite& ow) {
            return ow.id == context->msg.guild_id && ow.deny.has(dpp::permissions::p_send_messages);
        });
        if (it == overwrites.end())
            co_return dppcmd::command_result::from_error(Responses::ChannelNotChilled);

        MongoManager::deleteChill(context->msg.channel_id, context->msg.guild_id);
        it->deny.remove(dpp::permissions::p_send_messages);

        dpp::confirmation_callback_t conf = co_await cluster->co_channel_edit_permissions(*channel, it->id, it->allow, it->deny, false);
        if (conf.is_error())
            co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Unchill", conf.get_error().human_readable));

        co_return dppcmd::command_result::from_success(Responses::ChannelThawed);
    }

    co_return dppcmd::command_result::from_error(Responses::GetChannelFailed);
}

dpp::task<dppcmd::command_result> Moderation::unmute(const dpp::guild_member& member)
{
    if (!member.is_communication_disabled())
        co_return dppcmd::command_result::from_error(std::format(Responses::UserNotMuted, member.get_mention()));

    dpp::confirmation_callback_t conf = co_await cluster->co_guild_member_timeout_remove(member.guild_id, member.user_id);
    if (conf.is_error())
        co_return dppcmd::command_result::from_error(std::format(Responses::ActionFailed, "Unmute", conf.get_error().human_readable));

    co_return dppcmd::command_result::from_success(std::format(Responses::UnmutedUser, member.get_mention()));
}
