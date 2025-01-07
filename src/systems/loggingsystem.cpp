#include "loggingsystem.h"
#include "cache.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/mongomanager.h"
#include "dppcmd/extensions/cache.h"
#include "dppcmd/utils/join.h"
#include "entities/embed.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>

namespace LoggingSystem
{
    constexpr std::string_view UpdateMessage = "*(If nothing appears here, then this is likely a permission update)*\n";

    std::string describeActions(std::span<const dpp::automod_action> actions)
    {
        return dppcmd::utility::join(actions, '\n', [](const dpp::automod_action& action) {
            std::string actionDesc;

            switch (action.type)
            {
            case dpp::amod_action_block_message:
                actionDesc = "Block flagged messages";
                break;
            case dpp::amod_action_send_alert:
                actionDesc = "Send an alert containing the flagged message to "
                    + dpp::utility::channel_mention(action.channel_id);
                break;
            case dpp::amod_action_timeout:
                actionDesc = "Timeout member for " + RR::utility::formatSeconds(action.duration_seconds);
                break;
            default:
                actionDesc = "Do unknown action";
                break;
            }

            if (!action.custom_message.empty())
                actionDesc += " and respond with \"" + action.custom_message + "\"";

            return actionDesc;
        });
    }

    std::string presetToString(dpp::automod_preset_type preset)
    {
        switch (preset)
        {
        case dpp::amod_preset_profanity: return "Profanity";
        case dpp::amod_preset_sexual_content: return "Sexual Content";
        case dpp::amod_preset_slurs: return "Slurs";
        default: return "Unknown Preset";
        }
    }

    std::string triggerToString(dpp::automod_trigger_type trigger)
    {
        switch (trigger)
        {
        case dpp::amod_type_harmful_link: return "Harmful Link";
        case dpp::amod_type_keyword: return "Keyword";
        case dpp::amod_type_keyword_preset: return "Keyword Preset";
        case dpp::amod_type_mention_spam: return "Mention Spam";
        case dpp::amod_type_spam: return "Spam";
        default: return "Unknown Trigger";
        }
    }

    dpp::task<void> writeToLogs(dpp::cluster* cluster, const dpp::guild* guild, dpp::embed&& embed)
    {
        embed.color = dpp::colors::blue;
        if (!embed.fields.empty() && embed.fields.back().name == "\u200b")
            embed.fields.pop_back();

        DbConfigChannels channels = MongoManager::fetchChannelConfig(guild->id);
        if (std::ranges::contains(guild->channels, (dpp::snowflake)channels.logsChannel))
            co_await cluster->co_message_create(dpp::message(channels.logsChannel, std::move(embed)));
    }

    dpp::task<void> on_automod_rule_create(dpp::cluster* cluster, const dpp::automod_rule_create_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.created.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_author(event.created.guild_id, event.created.creator_id)
            .set_description("AutoMod Rule Created")
            .add_field("Name", event.created.name, true)
            .add_field("Enabled", event.created.enabled, true)
            .add_field("Trigger", triggerToString(event.created.trigger_type), true)
            .add_field("Keywords",
                dppcmd::utility::join(event.created.trigger_metadata.keywords, ", "), true)
            .add_field("Regex Patterns",
                dppcmd::utility::join(event.created.trigger_metadata.regex_patterns, ", "), true)
            .add_field("Presets",
                dppcmd::utility::join(event.created.trigger_metadata.presets, ", ", &presetToString), true)
            .add_field("Whitelist",
                dppcmd::utility::join(event.created.trigger_metadata.allow_list, ", "), true)
            .add_field("Exempt Channels",
                dppcmd::utility::join(event.created.exempt_channels, ", ", &dpp::utility::channel_mention), true)
            .add_field("Exempt Roles",
                dppcmd::utility::join(event.created.exempt_roles, ", ", &dpp::utility::role_mention), true)
            .add_field("Actions", describeActions(event.created.actions), true);

        if (event.created.trigger_metadata.mention_total_limit)
            embed.add_field("Mention Limit", event.created.trigger_metadata.mention_total_limit, true);

        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_automod_rule_delete(dpp::cluster* cluster, const dpp::automod_rule_delete_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.deleted.guild_id);
        if (!guild)
            co_return;

        dpp::embed embed = dpp::embed()
            .set_description("AutoMod Rule Deleted")
            .add_field("Name", event.deleted.name);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_automod_rule_update(dpp::cluster* cluster, const dpp::automod_rule_update_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.updated.guild_id);
        if (!guild)
            co_return;

        if (dpp::automod_rule* before = RR::find_automod_rule(event.updated.id))
        {
            std::string triggerBefore = triggerToString(before->trigger_type);
            std::string triggerAfter = triggerToString(event.updated.trigger_type);
            std::string keywordsBefore = dppcmd::utility::join(before->trigger_metadata.keywords, ", ");
            std::string keywordsAfter = dppcmd::utility::join(event.updated.trigger_metadata.keywords, ", ");
            std::string patternsBefore = dppcmd::utility::join(before->trigger_metadata.regex_patterns, ", ");
            std::string patternsAfter = dppcmd::utility::join(event.updated.trigger_metadata.regex_patterns, ", ");
            std::string presetsBefore = dppcmd::utility::join(before->trigger_metadata.presets, ", ", &presetToString);
            std::string presetsAfter = dppcmd::utility::join(event.updated.trigger_metadata.presets, ", ", &presetToString);
            std::string whitelistBefore = dppcmd::utility::join(before->trigger_metadata.allow_list, ", ");
            std::string whitelistAfter = dppcmd::utility::join(event.updated.trigger_metadata.allow_list, ", ");
            std::string exemptChannelsBefore = dppcmd::utility::join(
                before->exempt_channels, ", ", &dpp::utility::channel_mention);
            std::string exemptChannelsAfter = dppcmd::utility::join(
                event.updated.exempt_channels, ", ", &dpp::utility::channel_mention);
            std::string exemptRolesBefore = dppcmd::utility::join(
                before->exempt_roles, ", ", &dpp::utility::role_mention);
            std::string exemptRolesAfter = dppcmd::utility::join(
                event.updated.exempt_roles, ", ", &dpp::utility::role_mention);
            std::string actionsBefore = describeActions(before->actions);
            std::string actionsAfter = describeActions(event.updated.actions);

            RR::embed embed = RR::embed()
                .set_title("AutoMod Rule Updated")
                .add_comparison_field("Name", before->name, event.updated.name)
                .add_comparison_field("Enabled", before->enabled, event.updated.enabled)
                .add_comparison_field("Trigger", triggerBefore, triggerAfter)
                .add_comparison_field("Keywords", keywordsBefore, keywordsAfter)
                .add_comparison_field("Regex Patterns", patternsBefore, patternsAfter)
                .add_comparison_field("Presets", presetsBefore, presetsAfter)
                .add_comparison_field("Whitelist", whitelistBefore, whitelistAfter)
                .add_comparison_field("Exempt Channels", exemptChannelsBefore, exemptChannelsAfter)
                .add_comparison_field("Exempt Roles", exemptRolesBefore, exemptRolesAfter)
                .add_comparison_field("Actions", actionsBefore, actionsAfter);

            if (int limitBefore = before->trigger_metadata.mention_total_limit,
                limitAfter = event.updated.trigger_metadata.mention_total_limit;
                limitBefore > 0 || limitAfter > 0)
            {
                embed.add_comparison_field("Mention Limit", limitBefore, limitAfter);
            }

            co_await writeToLogs(cluster, guild, std::move(embed));
            *before = event.updated;
        }
    }

    dpp::task<void> on_channel_create(dpp::cluster* cluster, const dpp::channel_create_t& event)
    {
        dpp::embed embed = dpp::embed()
            .set_title("Channel Created")
            .set_description(event.created->get_mention());
        co_await writeToLogs(cluster, event.creating_guild, std::move(embed));
    }

    dpp::task<void> on_channel_delete(dpp::cluster* cluster, const dpp::channel_delete_t& event)
    {
        dpp::embed embed = dpp::embed()
            .set_title("Channel Deleted")
            .set_description(event.deleted.name);
        co_await writeToLogs(cluster, event.deleting_guild, std::move(embed));
    }

    dpp::task<void> on_channel_update(dpp::cluster* cluster, const dpp::channel_update_t& event)
    {
        RR::embed embed = RR::embed()
            .set_title("Channel Updated")
            .set_description(event.updated->get_mention().insert(0, UpdateMessage));

        dpp::channel* before = dpp::find_channel(event.updated->id);
        if (!before)
        {
            co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
            co_return;
        }

        // we don't care about position changes, those are boring
        if (before->position != event.updated->position)
            co_return;

        embed.add_comparison_field("Name", before->name, event.updated->name);
        embed.add_comparison_field("Topic", before->topic, event.updated->topic);

        if (event.updated->is_forum())
        {
            auto tagNameFilter = [](const dpp::forum_tag& tag) { return tag.name; };
            std::string beforeTags = dppcmd::utility::join(before->available_tags, ", ", tagNameFilter);
            std::string afterTags = dppcmd::utility::join(event.updated->available_tags, ", ", tagNameFilter);
            embed.add_comparison_field("Tags", beforeTags, afterTags);
        }
        else if (event.updated->is_voice_channel())
        {
            embed.add_comparison_field("Bitrate", before->bitrate, event.updated->bitrate);
            embed.add_comparison_field("Region", before->rtc_region, event.updated->rtc_region);
            embed.add_comparison_field("User Limit", before->user_limit, event.updated->user_limit);
        }

        co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
    }

    dpp::task<void> on_guild_ban_add(dpp::cluster* cluster, const dpp::guild_ban_add_t& event)
    {
        RR::embed embed = RR::embed()
            .set_author(event.banning_guild->id, event.banned.id)
            .set_title("User Banned");
        co_await writeToLogs(cluster, event.banning_guild, std::move(embed));
    }

    dpp::task<void> on_guild_ban_remove(dpp::cluster* cluster, const dpp::guild_ban_remove_t& event)
    {
        RR::embed embed = RR::embed()
            .set_author(event.unbanning_guild->id, event.unbanned.id)
            .set_title("User Unbanned");
        co_await writeToLogs(cluster, event.unbanning_guild, std::move(embed));
    }

    dpp::task<void> on_guild_member_add(dpp::cluster* cluster, const dpp::guild_member_add_t& event)
    {
        if (dpp::user* user = event.added.get_user())
        {
            RR::embed embed = RR::embed()
                .set_author(event.added, user)
                .set_title("User Joined");
            co_await writeToLogs(cluster, event.adding_guild, std::move(embed));
        }
    }

    dpp::task<void> on_guild_member_remove(dpp::cluster* cluster, const dpp::guild_member_remove_t& event)
    {
        if (auto member = dppcmd::find_guild_member_opt(event.guild_id, event.removed.id))
        {
            RR::embed embed = RR::embed()
                .set_author(member.value(), event.removed)
                .set_title("User Left");
            co_await writeToLogs(cluster, event.removing_guild, std::move(embed));
        }
    }

    dpp::task<void> on_guild_member_update(dpp::cluster* cluster, const dpp::guild_member_update_t& event)
    {
        if (auto before = dppcmd::find_guild_member_opt(event.updated.guild_id, event.updated.user_id))
        {
            if (before->get_nickname() == event.updated.get_nickname() &&
                std::ranges::equal(before->get_roles(), event.updated.get_roles()))
            {
                co_return;
            }

            if (dpp::user* user = event.updated.get_user())
            {
                std::string rolesBefore = dppcmd::utility::join(
                    before->get_roles(), ' ', &dpp::utility::channel_mention);
                std::string rolesAfter = dppcmd::utility::join(
                    event.updated.get_roles(), ' ', &dpp::utility::channel_mention);

                RR::embed embed = RR::embed()
                    .set_author(event.updated, user)
                    .set_title("User Updated")
                    .add_comparison_field("Nickname", before->get_nickname(), event.updated.get_nickname())
                    .add_comparison_field("Roles", rolesBefore, rolesAfter);
                co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
            }
        }
    }

    dpp::task<void> on_guild_role_create(dpp::cluster* cluster, const dpp::guild_role_create_t& event)
    {
        dpp::embed embed = dpp::embed()
            .set_title("Role Created")
            .add_field("Name", event.created->name, true)
            .add_field("Color", std::format("{:#06x}", event.created->colour), true);
        co_await writeToLogs(cluster, event.creating_guild, std::move(embed));
    }

    dpp::task<void> on_guild_role_delete(dpp::cluster* cluster, const dpp::guild_role_delete_t& event)
    {
        dpp::embed embed = dpp::embed()
            .set_title("Role Deleted")
            .set_description(event.deleted->name);
        co_await writeToLogs(cluster, event.deleting_guild, std::move(embed));
    }

    dpp::task<void> on_guild_role_update(dpp::cluster* cluster, const dpp::guild_role_update_t& event)
    {
        RR::embed embed = RR::embed()
            .set_title("Role Updated")
            .set_description(event.updated->get_mention().insert(0, UpdateMessage));

        dpp::role* before = dpp::find_role(event.updated->id);
        if (!before)
        {
            co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
            co_return;
        }

        embed.add_comparison_field("Name", before->name, event.updated->name);
        embed.add_comparison_field("Role",
            std::format("{:#06x}", before->colour),
            std::format("{:#06x}", event.updated->colour));

        co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
    }

    dpp::task<void> on_guild_scheduled_event_cancel(dpp::cluster* cluster,
        const dpp::guild_scheduled_event_delete_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.deleted.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_title("Event Cancelled")
            .add_field("Name", event.deleted.name, true)
            .add_field("Description", event.deleted.description, true)
            .add_field("Location", event.deleted.entity_metadata.location, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_scheduled_event_complete(dpp::cluster* cluster,
        const dpp::guild* guild, const dpp::scheduled_event& completed)
    {
        RR::embed embed = RR::embed()
            .set_title("Event Ended")
            .add_field("Name", completed.name, true)
            .add_field("Description", completed.description, true)
            .add_field("Location", completed.entity_metadata.location, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_scheduled_event_create(dpp::cluster* cluster,
        const dpp::guild_scheduled_event_create_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.created.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_author(event.created.guild_id, event.created.creator_id)
            .set_title("Event Created")
            .add_field("Name", event.created.name, true)
            .add_field("Description", event.created.description, true)
            .add_field("Location", event.created.entity_metadata.location, true)
            .add_field("Start Time", dpp::utility::timestamp(event.created.scheduled_start_time), true)
            .add_field("End Time", dpp::utility::timestamp(event.created.scheduled_end_time), true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_scheduled_event_start(dpp::cluster* cluster,
        const dpp::guild* guild, const dpp::scheduled_event& started)
    {
        RR::embed embed = RR::embed()
            .set_title("Event Started")
            .add_field("Name", started.name)
            .add_field("Description", started.description)
            .add_field("Location", started.entity_metadata.location);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_scheduled_event_update(dpp::cluster* cluster, const dpp::guild* guild,
        const dpp::scheduled_event& before, const dpp::scheduled_event& after)
    {
        std::string beforeStart = dpp::utility::timestamp(before.scheduled_start_time);
        std::string afterStart = dpp::utility::timestamp(after.scheduled_start_time);
        std::string beforeEnd = dpp::utility::timestamp(before.scheduled_end_time);
        std::string afterEnd = dpp::utility::timestamp(after.scheduled_end_time);

        RR::embed embed = RR::embed()
            .set_title("Event Updated")
            .set_description(after.name)
            .add_comparison_field("Name", before.name, after.name)
            .add_comparison_field("Description", before.description, after.description)
            .add_comparison_field("Location", before.entity_metadata.location, after.entity_metadata.location)
            .add_comparison_field("Start Time", beforeStart, afterStart)
            .add_comparison_field("End Time", beforeEnd, afterEnd);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_sticker_create(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker)
    {
        RR::embed embed = RR::embed()
            .set_author(guild->id, sticker.sticker_user.id)
            .set_title("Sticker Created")
            .set_image(sticker.get_url())
            .add_field("Name", sticker.name, true)
            .add_field("Description", sticker.description, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_sticker_delete(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker)
    {
        RR::embed embed = RR::embed()
            .set_author(guild->id, sticker.sticker_user.id)
            .set_title("Sticker Deleted")
            .set_image(sticker.get_url())
            .add_field("Name", sticker.name, true)
            .add_field("Description", sticker.description, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_sticker_update(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker)
    {
        RR::embed embed = RR::embed()
            .set_author(guild->id, sticker.sticker_user.id)
            .set_title("Sticker Updated")
            .set_image(sticker.get_url())
            .add_field("Name", sticker.name, true)
            .add_field("Description", sticker.description, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_guild_update(dpp::cluster* cluster, const dpp::guild_update_t& event)
    {
        if (dpp::guild* before = dpp::find_guild(event.updated->id))
        {
            if (before->name == event.updated->name && before->description == event.updated->description)
                co_return;

            RR::embed embed = RR::embed()
                .set_title("Guild Updated")
                .add_comparison_field("Name", before->name, event.updated->name)
                .add_comparison_field("Description", before->description, event.updated->description);

            co_await writeToLogs(cluster, event.updated, std::move(embed));
            *before = *event.updated;
        }
    }

    dpp::task<void> on_invite_create(dpp::cluster* cluster, const dpp::invite_create_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.created_invite.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_author(event.created_invite.guild_id, event.created_invite.inviter_id)
            .set_title("Invite Created")
            .add_field("Channel", event.created_invite.destination_channel.get_mention(), true)
            .add_field("Code", event.created_invite.code, true)
            .add_field("Expires At", dpp::utility::timestamp(event.created_invite.expires_at), true)
            .add_field("Max Age", std::format("{} Days", event.created_invite.max_age), true)
            .add_field("Max Uses", event.created_invite.max_uses, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_invite_delete(dpp::cluster* cluster, const dpp::invite_delete_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.deleted_invite.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_author(event.deleted_invite.guild_id, event.deleted_invite.inviter_id)
            .set_title("Invite Deleted")
            .add_field("Channel", dpp::utility::channel_mention(event.deleted_invite.channel_id), true)
            .add_field("Code", event.deleted_invite.code, true)
            .add_field("Uses", event.deleted_invite.uses, true);
        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_message_delete(dpp::cluster* cluster, const dpp::message_delete_t& event)
    {
        dpp::message* message = RR::find_message(event.id);
        if (!message || message->author.id == cluster->me.id)
            co_return;

        const dpp::guild* guild = dpp::find_guild(event.guild_id);
        if (!guild)
            co_return;

        RR::embed embed = RR::embed()
            .set_color(dpp::colors::blue)
            .set_title("Message Deleted in " + dpp::utility::channel_mention(event.channel_id))
            .add_field("Content", message->content, true)
            .set_footer(std::format("ID: {} • Any attachments and embeds are below!", (uint64_t)event.id));

        dpp::message outMessage(event.channel_id, std::move(embed));
        outMessage.attachments = std::move(message->attachments);
        std::ranges::move(message->embeds, std::back_inserter(outMessage.embeds));

        DbConfigChannels channels = MongoManager::fetchChannelConfig(guild->id);
        if (std::ranges::contains(guild->channels, (dpp::snowflake)channels.logsChannel))
            co_await cluster->co_message_create(std::move(outMessage));
    }

    dpp::task<void> on_message_delete_bulk(dpp::cluster* cluster, const dpp::guild* guild,
                                        const std::vector<dpp::message*>& messages)
    {
        std::string messageLog = dppcmd::utility::join(messages, '\n', [](const dpp::message* msg) {
            std::chrono::utc_clock::time_point tp(std::chrono::seconds(msg->sent));
            return std::format("{0} @ {1:%F}T{1:%T%z}: {2}", msg->author.global_name, tp, msg->content);
        });

        dpp::http_request_completion_t res = co_await cluster->co_request(
            "https://hst.sh/documents", dpp::m_post, messageLog);

        dpp::embed embed = dpp::embed().set_title(std::format("{} Messages Purged", messages.size()));
        if (res.status == 200)
        {
            std::string pasteKey = nlohmann::json::parse(res.body, nullptr, false)["key"].template get<std::string>();
            embed.set_description("See them [here](" + pasteKey + ")");
        }
        else
        {
            embed.set_description("But failed to upload a log of the messages :(");
        }

        co_await writeToLogs(cluster, guild, std::move(embed));
    }

    dpp::task<void> on_message_update(dpp::cluster* cluster, const dpp::message_update_t& event)
    {
        if (dpp::message* before = RR::find_message(event.msg.id))
        {
            if (before->content != event.msg.content)
            {
                if (const dpp::guild* guild = dpp::find_guild(event.msg.guild_id))
                {
                    RR::embed embed = RR::embed()
                        .set_author(event.msg.guild_id, event.msg.author.id)
                        .set_title("Message Updated in " + dpp::utility::channel_mention(event.msg.channel_id))
                        .set_description(std::format("[Jump]({})", event.msg.get_url()))
                        .add_comparison_field("Content", before->content, event.msg.content);
                    co_await writeToLogs(cluster, guild, std::move(embed));
                }
            }

            *before = event.msg;
        }
    }

    dpp::task<void> on_thread_create(dpp::cluster* cluster, const dpp::thread_create_t& event)
    {
        RR::embed embed = RR::embed()
            .set_author(event.created.guild_id, event.created.owner_id)
            .set_title("Thread Created")
            .add_field("Channel", dpp::utility::channel_mention(event.created.parent_id))
            .add_field("Name", event.created.name);
        co_await writeToLogs(cluster, event.creating_guild, std::move(embed));
    }

    dpp::task<void> on_thread_delete(dpp::cluster* cluster, const dpp::thread_delete_t& event)
    {
        RR::embed embed = RR::embed()
            .set_title("Thread Deleted")
            .add_field("Channel", dpp::utility::channel_mention(event.deleted.parent_id))
            .add_field("Name", event.deleted.name);
        co_await writeToLogs(cluster, event.deleting_guild, std::move(embed));
    }

    dpp::task<void> on_thread_update(dpp::cluster* cluster, const dpp::thread_update_t& event)
    {
        if (dpp::thread* before = RR::find_thread(event.updated.id))
        {
            RR::embed embed = RR::embed()
                .set_title("Thread Updated")
                .set_description(event.updated.get_mention().insert(0, UpdateMessage))
                .add_comparison_field("Name", before->name, event.updated.name)
                .add_comparison_field("Member Count", before->member_count, event.updated.member_count)
                .add_comparison_field("Position", before->position, event.updated.position);

            co_await writeToLogs(cluster, event.updating_guild, std::move(embed));
            *before = event.updated;
        }
    }
}
