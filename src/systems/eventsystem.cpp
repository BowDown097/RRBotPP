#include "eventsystem.h"
#include "cache.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/commands/commandinfo.h"
#include "dppcmd/commands/exceptions.h"
#include "dppcmd/extensions/cache.h"
#include "dppcmd/modules/modulebase.h"
#include "dppcmd/services/moduleservice.h"
#include "systems/filtersystem.h"
#include "systems/itemsystem.h"
#include "systems/loggingsystem.h"
#include "utils/dpp.h"
#include "utils/random.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>

inline bool operator==(const dpp::sticker& lhs, const dpp::sticker& rhs)
{
    return lhs.name == rhs.name &&
           lhs.description == rhs.description &&
           lhs.format_type == rhs.format_type &&
           lhs.id == rhs.id &&
           lhs.pack_id == rhs.pack_id &&
           lhs.type == rhs.type &&
           lhs.sort_value == rhs.sort_value &&
           lhs.available == rhs.available &&
           lhs.tags == rhs.tags;
}

dpp::task<void> on_automod_rule_create(dpp::cluster* cluster, const dpp::automod_rule_create_t& event)
{
    co_await LoggingSystem::on_automod_rule_create(cluster, event);
    RR::get_automod_rule_cache()->store(new dpp::automod_rule(event.created));
}

dpp::task<void> on_automod_rule_delete(dpp::cluster* cluster, const dpp::automod_rule_delete_t& event)
{
    co_await LoggingSystem::on_automod_rule_delete(cluster, event);
    RR::get_automod_rule_cache()->remove(RR::find_automod_rule(event.deleted.id));
}

dpp::task<void> on_guild_create(dpp::cluster* cluster, const dpp::guild_create_t& event)
{
    if (!event.created)
        co_return;

    // cache stickers
    if (dpp::confirmation_callback_t conf = co_await cluster->co_guild_stickers_get(event.created->id); !conf.is_error())
        for (const auto& [_, sticker] : conf.get<dpp::sticker_map>())
            RR::get_sticker_cache()->store(new dpp::sticker(sticker));

    // send join message if needed
    if (event.created->is_unavailable())
        if (dpp::channel* defaultChannel = RR::utility::getDefaultChannel(event.created->channels, &cluster->me))
            co_await cluster->co_message_create(dpp::message(defaultChannel->id, Responses::JoinMessage));
}

dpp::task<void> on_guild_scheduled_event_create(dpp::cluster* cluster, const dpp::guild_scheduled_event_create_t& event)
{
    co_await LoggingSystem::on_guild_scheduled_event_create(cluster, event);
    RR::get_scheduled_event_cache()->store(new dpp::scheduled_event(event.created));
}

dpp::task<void> on_guild_scheduled_event_delete(dpp::cluster* cluster, const dpp::guild_scheduled_event_delete_t& event)
{
    co_await LoggingSystem::on_guild_scheduled_event_cancel(cluster, event);
    RR::get_scheduled_event_cache()->remove(RR::find_scheduled_event(event.deleted.id));
}

dpp::task<void> on_guild_scheduled_event_update(dpp::cluster* cluster, const dpp::guild_scheduled_event_update_t& event)
{
    const dpp::guild* guild = dpp::find_guild(event.updated.guild_id);
    if (!guild)
        co_return;

    if (dpp::scheduled_event* before = RR::find_scheduled_event(event.updated.id))
    {
        if (before->status != dpp::es_completed && event.updated.status == dpp::es_completed)
            co_await LoggingSystem::on_guild_scheduled_event_complete(cluster, guild, event.updated);
        else if (before->status != dpp::es_active && event.updated.status == dpp::es_active)
            co_await LoggingSystem::on_guild_scheduled_event_start(cluster, guild, event.updated);
        else
            co_await LoggingSystem::on_guild_scheduled_event_update(cluster, guild, *before, event.updated);
        *before = event.updated;
    }
}

dpp::task<void> on_guild_stickers_update(dpp::cluster* cluster, const dpp::guild_stickers_update_t& event)
{
    auto createdFilter = [](const dpp::sticker& s) { return !RR::find_sticker(s.id); };
    auto deletedFilter = [&event](const std::pair<dpp::snowflake, dpp::sticker*>& p) {
        return !std::ranges::any_of(event.stickers, [id = p.first](const dpp::sticker& s) { return s.id == id; });
    };
    auto updatedFilter = [](const dpp::sticker& s) {
        const dpp::sticker* gs = RR::find_sticker(s.id);
        return gs && *gs != s;
    };

    // AFAIK, multiple kinds of updates can't happen at the same time, so else ifs should work.
    std::unordered_map<dpp::snowflake, dpp::sticker*>& cached = RR::get_sticker_cache()->get_container();
    if (auto created = std::views::filter(event.stickers, createdFilter); !std::ranges::empty(created))
    {
        for (const dpp::sticker& sticker : created)
        {
            co_await LoggingSystem::on_guild_sticker_create(cluster, event.updating_guild, sticker);
            RR::get_sticker_cache()->store(new dpp::sticker(sticker));
        }
    }
    else if (auto deleted = std::views::filter(cached, deletedFilter); !std::ranges::empty(deleted))
    {
        for (auto& [_, sticker] : deleted)
        {
            co_await LoggingSystem::on_guild_sticker_delete(cluster, event.updating_guild, *sticker);
            RR::get_sticker_cache()->remove(sticker);
        }
    }
    else if (auto updated = std::views::filter(event.stickers, updatedFilter); !std::ranges::empty(updated))
    {
        for (const dpp::sticker& sticker : updated)
        {
            co_await LoggingSystem::on_guild_sticker_update(cluster, event.updating_guild, sticker);
            if (dpp::sticker* stickerPtr = RR::find_sticker(sticker.id))
                *stickerPtr = sticker;
        }
    }
}

dpp::task<void> on_message_create(dpp::cluster* cluster, dppcmd::module_service* modules, const dpp::message_create_t& event)
{
    std::string_view content = event.msg.content;
    if (content.empty() || event.msg.author.is_bot())
        co_return;

    DbConfigChannels channels = MongoManager::fetchChannelConfig(event.msg.guild_id);
    DbConfigMisc misc = MongoManager::fetchMiscConfig(event.msg.guild_id);

    if (!channels.noFilterChannels.contains(event.msg.channel_id) &&
        co_await FilterSystem::messageIsBad(event.msg, cluster, misc))
    {
        co_await cluster->co_message_delete(event.msg.id, event.msg.channel_id);
        co_return;
    }

    RR::get_message_cache()->store(new dpp::message(event.msg));

    DbUser user = MongoManager::fetchUser(event.msg.author.id, event.msg.guild_id);
    if (content.starts_with(modules->config().command_prefix))
    {
        try
        {
            std::vector<const dppcmd::command_info*> cmds = modules->search_command(
                content.substr(1, content.find(modules->config().separator_char) - 1));
            if (cmds.empty())
                co_return;

            if (user.usingSlots)
            {
                event.reply(Responses::YouAreGambling);
                co_return;
            }

            const dppcmd::command_info* cmd = cmds.front();
            constexpr std::array exemptModules = { "Administration", "BotOwner", "Config", "Moderation" };

            if (!std::ranges::contains(exemptModules, cmd->module->name()) &&
                !channels.whitelistedChannels.empty() &&
                !channels.whitelistedChannels.contains(event.msg.channel_id))
            {
                event.reply(Responses::CommandsDisabled);
                co_return;
            }

            DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
            if (globalConfig.bannedUsers.contains(event.msg.author.id))
            {
                event.reply(Responses::BannedFromBot);
                co_return;
            }
            if (globalConfig.disabledCommands.contains(cmd->name()) || misc.disabledCommands.contains(cmd->name()))
            {
                event.reply(Responses::CommandDisabled);
                co_return;
            }
            if (misc.disabledModules.contains(cmd->module->name()))
            {
                event.reply(Responses::ModuleDisabled);
                co_return;
            }

            dppcmd::command_result result = co_await modules->handle_message(&event);
            if (result.message().empty())
                co_return;

            std::optional<dppcmd::command_error> error = result.error();
            using CmdErr = dppcmd::command_error;
            if (result.success() || error == CmdErr::unsuccessful || error == CmdErr::unmet_precondition)
                event.reply(result.message());
        }
        catch (const dppcmd::bad_argument_count& ex)
        {
            std::vector<const dppcmd::command_info*> cmds = modules->search_command(ex.command());
            event.reply(!cmds.empty()
                ? std::format(Responses::BadArgCount, ex.target_arg_count(), cmds.front()->remarks)
                : std::format(Responses::ErrorOccurred, ex.what()));
        }
        catch (const dppcmd::bad_command_argument& ex)
        {
            if (ex.error() == dppcmd::command_error::multiple_matches)
            {
                event.reply(ex.message());
                co_return;
            }

            std::vector<const dppcmd::command_info*> cmds = modules->search_command(ex.command());
            event.reply(!cmds.empty()
                ? std::format(Responses::BadArgument, ex.arg(), ex.message(), cmds.front()->remarks)
                : std::format(Responses::ErrorOccurred, ex.what()));
        }
        catch (const std::exception& ex)
        {
            event.reply(std::format(Responses::ErrorOccurred, ex.what()));
        }
    }
    else
    {
        if (user.messageCashCooldown <= 0)
        {
            user.messageCashCooldown = RR::utility::unixTimestamp(Constants::MessageCashCooldown);
        }
        else if (user.messageCashCooldown <= RR::utility::unixTimestamp())
        {
            auto member = dppcmd::find_guild_member_opt(event.msg.guild_id, event.msg.author.id);
            if (!member)
                co_return;

            co_await user.setCash(member.value(), user.cash + Constants::MessageCash, cluster);
            user.modCooldown(user.messageCashCooldown = Constants::MessageCashCooldown, member.value());

            if (!misc.dropsDisabled && RR::utility::random(70) == 1)
                ItemSystem::giveCollectible("Bank Cheque", &event, user);

            if (user.cash >= 1000000.0L && !user.hasReachedAMilli)
            {
                user.hasReachedAMilli = true;
                ItemSystem::giveCollectible("V Card", &event, user);
            }
        }

        MongoManager::updateUser(user);
    }
}

dpp::task<void> on_message_delete(dpp::cluster* cluster, const dpp::message_delete_t& event)
{
    co_await LoggingSystem::on_message_delete(cluster, event);
    RR::get_message_cache()->remove(RR::find_message(event.id));
}

dpp::task<void> on_message_delete_bulk(dpp::cluster* cluster, const dpp::message_delete_bulk_t& event)
{
    auto messages = event.deleted
        | std::views::transform([](dpp::snowflake id) { return RR::find_message(id); })
        | std::views::filter([](const dpp::message* m) { return m != nullptr; });

    co_await LoggingSystem::on_message_delete_bulk(cluster, event.deleting_guild, std::ranges::to<std::vector>(messages));
    std::ranges::for_each(messages, [](dpp::message* msg) { RR::get_message_cache()->remove(msg); });
}

dpp::task<void> on_message_update(dpp::cluster* cluster, const dpp::message_update_t& event)
{
    if (event.msg.content.empty() || event.msg.author.is_bot())
        co_return;

    DbConfigChannels channels = MongoManager::fetchChannelConfig(event.msg.guild_id);
    DbConfigMisc misc = MongoManager::fetchMiscConfig(event.msg.guild_id);

    if (!channels.noFilterChannels.contains(event.msg.channel_id) &&
        co_await FilterSystem::messageIsBad(event.msg, cluster, misc))
    {
        co_await cluster->co_message_delete(event.msg.id, event.msg.channel_id);
    }
}

dpp::task<void> on_thread_create(dpp::cluster* cluster, const dpp::thread_create_t& event)
{
    co_await LoggingSystem::on_thread_create(cluster, event);
    RR::get_thread_cache()->store(new dpp::thread(event.created));
}

dpp::task<void> on_thread_delete(dpp::cluster* cluster, const dpp::thread_delete_t& event)
{
    co_await LoggingSystem::on_thread_delete(cluster, event);
    RR::get_thread_cache()->remove(RR::find_thread(event.deleted.id));
}

void EventSystem::connectEvents(dpp::cluster* cluster, dppcmd::module_service* modules)
{
    #define EVENT_CONNECTION(event) cluster->event(std::bind_front(&event, cluster))
    #define LOGGING_CONNECTION(event) cluster->event(std::bind_front(&LoggingSystem::event, cluster))

    EVENT_CONNECTION(on_automod_rule_create);
    EVENT_CONNECTION(on_automod_rule_delete);
    EVENT_CONNECTION(on_guild_create);
    EVENT_CONNECTION(on_guild_scheduled_event_create);
    EVENT_CONNECTION(on_guild_scheduled_event_delete);
    EVENT_CONNECTION(on_guild_scheduled_event_update);
    EVENT_CONNECTION(on_guild_stickers_update);
    EVENT_CONNECTION(on_message_delete);
    EVENT_CONNECTION(on_message_delete_bulk);
    EVENT_CONNECTION(on_message_update);
    EVENT_CONNECTION(on_thread_create);
    EVENT_CONNECTION(on_thread_delete);

    cluster->on_log(dpp::utility::cout_logger());
    cluster->on_message_create(std::bind_front(&on_message_create, cluster, modules));

    // next: on_stage_*, on_thread_members_update
    LOGGING_CONNECTION(on_automod_rule_update);
    LOGGING_CONNECTION(on_channel_create);
    LOGGING_CONNECTION(on_channel_delete);
    LOGGING_CONNECTION(on_channel_update);
    LOGGING_CONNECTION(on_guild_ban_add);
    LOGGING_CONNECTION(on_guild_ban_remove);
    LOGGING_CONNECTION(on_guild_member_add);
    LOGGING_CONNECTION(on_guild_member_remove);
    LOGGING_CONNECTION(on_guild_member_update);
    LOGGING_CONNECTION(on_guild_role_create);
    LOGGING_CONNECTION(on_guild_role_delete);
    LOGGING_CONNECTION(on_guild_role_update);
    LOGGING_CONNECTION(on_guild_update);
    LOGGING_CONNECTION(on_invite_create);
    LOGGING_CONNECTION(on_invite_delete);
    LOGGING_CONNECTION(on_message_update);
    LOGGING_CONNECTION(on_thread_update);
}
