#include "dpp.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>

namespace RR
{
    namespace utility
    {
        dpp::embed_author asEmbedAuthor(const dpp::guild_member& guildMember, const dpp::user* user, bool useNickname)
        {
            dpp::embed_author out;
            out.icon_url = getDisplayAvatar(guildMember, user);
            if (useNickname)
            {
                if (std::string nickname = guildMember.get_nickname(); !nickname.empty())
                    out.name = nickname;
                else
                    out.name = user->global_name;
            }
            else
            {
                out.name = user->global_name;
            }

            return out;
        }

        dpp::channel* getDefaultChannel(const std::vector<dpp::snowflake>& channelIds, const dpp::user* botUser)
        {
            auto channelFilter = [botUser](dpp::channel* c) {
                return c && c->get_user_permissions(botUser).can(dpp::p_view_channel) && c->is_text_channel();
            };

            auto validChannelsRange = channelIds
                | std::views::transform([](dpp::snowflake id) { return dpp::find_channel(id); })
                | std::views::filter(channelFilter);
            if (std::ranges::empty(validChannelsRange))
                return nullptr;

            std::vector<dpp::channel*> validChannels = std::ranges::to<std::vector>(validChannelsRange);
            std::ranges::sort(validChannels, {}, [](const dpp::channel* c) { return c->position; });
            return validChannels.front();
        }

        std::string getDisplayAvatar(const dpp::guild_member& guildMember, const dpp::user* user)
        {
            if (std::string guildAvatar = guildMember.get_avatar_url(); !guildAvatar.empty())
                return guildAvatar;
            else
                return user->get_avatar_url();
        }

        dpp::task<time_t> getJoinTime(dpp::cluster* cluster, dpp::guild* guild, dpp::snowflake userId)
        {
            auto memberFilter = [userId](const std::pair<dpp::snowflake, dpp::guild_member>& p) { return p.first == userId; };
            if (auto it = std::ranges::find_if(guild->members, memberFilter); it != guild->members.end())
                co_return it->second.joined_at;
            else if (auto memberConf = co_await cluster->co_guild_get_member(guild->id, userId); !memberConf.is_error())
                co_return memberConf.get<dpp::guild_member>().joined_at;
            co_return time_t{};
        }

        std::vector<dpp::snowflake> getLast14DaysMessages(const dpp::message_map& map)
        {
            long ts14DaysAgo = RR::utility::unixTimestamp(-RR::utility::secondsInDays(14));
            return map
                | std::views::filter([ts14DaysAgo](const auto& p) { return p.second.sent > ts14DaysAgo; })
                | std::views::keys
                | std::ranges::to<std::vector>();
        }

        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions)
        {
            constexpr std::array<std::pair<dpp::permissions, std::string_view>, 44> permissionsMap = {{
                { dpp::permissions::p_create_instant_invite, "Create Invite" },
                { dpp::permissions::p_kick_members, "Kick Members" },
                { dpp::permissions::p_ban_members, "Ban Members" },
                { dpp::permissions::p_administrator, "Administrator" },
                { dpp::permissions::p_manage_channels, "Manage Channels" },
                { dpp::permissions::p_manage_guild, "Manage Server" },
                { dpp::permissions::p_add_reactions, "Add Reactions" },
                { dpp::permissions::p_view_audit_log, "View Audit Log" },
                { dpp::permissions::p_priority_speaker, "Priority Speaker" },
                { dpp::permissions::p_stream, "Video" },
                { dpp::permissions::p_view_channel, "View Channel" },
                { dpp::permissions::p_send_messages, "Send Messages" },
                { dpp::permissions::p_send_tts_messages, "Send Text-to-Speech Messages" },
                { dpp::permissions::p_manage_messages, "Manage Messages" },
                { dpp::permissions::p_embed_links, "Embed Links" },
                { dpp::permissions::p_attach_files, "Attach Files" },
                { dpp::permissions::p_read_message_history, "Read Message History" },
                { dpp::permissions::p_mention_everyone, "Mention @everyone, @here, and All Roles" },
                { dpp::permissions::p_use_external_emojis, "Use External Emoji" },
                { dpp::permissions::p_view_guild_insights, "View Server Insights" },
                { dpp::permissions::p_connect, "Connect" },
                { dpp::permissions::p_speak, "Speak" },
                { dpp::permissions::p_mute_members, "Mute Members" },
                { dpp::permissions::p_move_members, "Move Members" },
                { dpp::permissions::p_use_vad, "Use Voice Activity" },
                { dpp::permissions::p_change_nickname, "Change Nickname" },
                { dpp::permissions::p_manage_nicknames, "Manage Nicknames" },
                { dpp::permissions::p_manage_roles, "Manage Roles" },
                { dpp::permissions::p_manage_webhooks, "Manage Webhooks" },
                { dpp::permissions::p_manage_emojis_and_stickers, "Manage Expressions" },
                { dpp::permissions::p_use_application_commands, "Use Application Commands" },
                { dpp::permissions::p_request_to_speak, "Request to Speak" },
                { dpp::permissions::p_manage_events, "Manage Events" },
                { dpp::permissions::p_manage_threads, "Manage Threads" },
                { dpp::permissions::p_create_public_threads, "Create Public Threads" },
                { dpp::permissions::p_create_private_threads, "Create Private Threads" },
                { dpp::permissions::p_use_external_stickers, "Use External Stickers" },
                { dpp::permissions::p_send_messages_in_threads, "Send Messages in Threads" },
                { dpp::permissions::p_use_embedded_activities, "Use Activities" },
                { dpp::permissions::p_moderate_members, "Timeout Members" },
                { dpp::permissions::p_view_creator_monetization_analytics, "View Creator Monetization Analytics" },
                { dpp::permissions::p_use_soundboard, "Use Soundboard" },
                { dpp::permissions::p_use_external_sounds, "Use External Sounds" },
                { dpp::permissions::p_send_voice_messages, "Send Voice Messages" }
            }};

            std::vector<std::pair<dpp::permissions, std::string>> out;
            for (const auto& [permission, name] : permissionsMap)
                if (permissions.has(permission))
                    out.emplace_back(permission, name);

            return out;
        }
    }
}
