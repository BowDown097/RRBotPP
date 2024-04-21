#include "rrutils.h"
#include <format>

namespace RR
{
    namespace utility
    {
        dpp::embed_author asEmbedAuthor(const dpp::guild_member& guildMember, const dpp::user* user,
                                        bool useNickname)
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

        std::optional<dpp::guild_member> findGuildMember(const dpp::snowflake guildId, const dpp::snowflake userId)
        {
            try
            {
                return dpp::find_guild_member(guildId, userId);
            }
            catch (const dpp::cache_exception& e)
            {
                return std::nullopt;
            }
        }

        std::string formatPair(const std::pair<std::string, std::string>& pair)
        {
            return "**" + pair.first + "**: " + pair.second;
        }

        // there is most likely an easier way to do this. that being said, if there is, i couldn't find it.
        std::string formatTimestamp(long timestamp)
        {
            auto duration = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(timestamp);
            if (auto hours = std::chrono::duration_cast<std::chrono::hours>(duration); hours.count() > 0)
            {
                auto mins = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
                auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - mins);
                return std::format("{}:{:02}:{:02}", hours.count(), mins.count(), secs.count());
            }
            else if (auto mins = std::chrono::duration_cast<std::chrono::minutes>(duration); mins.count() > 0)
            {
                auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration - mins);
                return std::format("{}:{:02}", mins.count(), secs.count());
            }

            auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
            return std::format("{}s", secs.count());
        }

        std::string getDisplayAvatar(const dpp::guild_member& guildMember, const dpp::user* user)
        {
            if (std::string guildAvatar = guildMember.get_avatar_url(); !guildAvatar.empty())
                return guildAvatar;
            else
                return user->get_avatar_url();
        }

        std::vector<std::pair<dpp::permissions, std::string>> permissionsToList(const dpp::permission& permissions)
        {
            constexpr std::array permissionsMap = {
                std::pair { dpp::permissions::p_create_instant_invite, "Create Invite" },
                std::pair { dpp::permissions::p_kick_members, "Kick Members" },
                std::pair { dpp::permissions::p_ban_members, "Ban Members" },
                std::pair { dpp::permissions::p_administrator, "Administrator" },
                std::pair { dpp::permissions::p_manage_channels, "Manage Channels" },
                std::pair { dpp::permissions::p_manage_guild, "Manage Server" },
                std::pair { dpp::permissions::p_add_reactions, "Add Reactions" },
                std::pair { dpp::permissions::p_view_audit_log, "View Audit Log" },
                std::pair { dpp::permissions::p_priority_speaker, "Priority Speaker" },
                std::pair { dpp::permissions::p_stream, "Video" },
                std::pair { dpp::permissions::p_view_channel, "View Channel" },
                std::pair { dpp::permissions::p_send_messages, "Send Messages" },
                std::pair { dpp::permissions::p_send_tts_messages, "Send Text-to-Speech Messages" },
                std::pair { dpp::permissions::p_manage_messages, "Manage Messages" },
                std::pair { dpp::permissions::p_embed_links, "Embed Links" },
                std::pair { dpp::permissions::p_attach_files, "Attach Files" },
                std::pair { dpp::permissions::p_read_message_history, "Read Message History" },
                std::pair { dpp::permissions::p_mention_everyone, "Mention @everyone, @here, and All Roles" },
                std::pair { dpp::permissions::p_use_external_emojis, "Use External Emoji" },
                std::pair { dpp::permissions::p_view_guild_insights, "View Server Insights" },
                std::pair { dpp::permissions::p_connect, "Connect" },
                std::pair { dpp::permissions::p_speak, "Speak" },
                std::pair { dpp::permissions::p_mute_members, "Mute Members" },
                std::pair { dpp::permissions::p_move_members, "Move Members" },
                std::pair { dpp::permissions::p_use_vad, "Use Voice Activity" },
                std::pair { dpp::permissions::p_change_nickname, "Change Nickname" },
                std::pair { dpp::permissions::p_manage_nicknames, "Manage Nicknames" },
                std::pair { dpp::permissions::p_manage_roles, "Manage Roles" },
                std::pair { dpp::permissions::p_manage_webhooks, "Manage Webhooks" },
                std::pair { dpp::permissions::p_manage_emojis_and_stickers, "Manage Expressions" },
                std::pair { dpp::permissions::p_use_application_commands, "Use Application Commands" },
                std::pair { dpp::permissions::p_request_to_speak, "Request to Speak" },
                std::pair { dpp::permissions::p_manage_events, "Manage Events" },
                std::pair { dpp::permissions::p_manage_threads, "Manage Threads" },
                std::pair { dpp::permissions::p_create_public_threads, "Create Public Threads" },
                std::pair { dpp::permissions::p_create_private_threads, "Create Private Threads" },
                std::pair { dpp::permissions::p_use_external_stickers, "Use External Stickers" },
                std::pair { dpp::permissions::p_send_messages_in_threads, "Send Messages in Threads" },
                std::pair { dpp::permissions::p_use_embedded_activities, "Use Activities" },
                std::pair { dpp::permissions::p_moderate_members, "Timeout Members" },
                std::pair { dpp::permissions::p_view_creator_monetization_analytics, "View Creator Monetization Analytics" },
                std::pair { dpp::permissions::p_use_soundboard, "Use Soundboard" },
                std::pair { dpp::permissions::p_use_external_sounds, "Use External Sounds" },
                std::pair { dpp::permissions::p_send_voice_messages, "Send Voice Messages" }
            };

            std::vector<std::pair<dpp::permissions, std::string>> out;
            for (const auto& [permission, name] : permissionsMap)
                if (permissions.has(permission))
                    out.emplace_back(permission, name);

            return out;
        }

        std::string sanitizeString(const std::string& str)
        {
            if (str.empty())
                return str;

            constexpr std::array escapedChars = { "\\*", "\\_", "\\`", "\\~", "\\>" };
            constexpr std::array sensitiveChars = { "*", "_", "`", "~", ">" };

            std::string sanitized(str);
            for (int i = 0; i < sensitiveChars.size(); ++i)
                strReplace(sanitized, sensitiveChars[i], escapedChars[i]);

            return sanitized;
        }

        void strReplace(std::string& str, std::string_view from, std::string_view to)
        {
            size_t pos = 0;
            while ((pos = str.find(from, pos)) != std::string::npos)
            {
                str.replace(pos, from.size(), to);
                pos += to.size();
            }
        }

        long unixTimeSecs(long offset)
        {
            return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())
                       .time_since_epoch().count() + offset;
        }
    }
}
