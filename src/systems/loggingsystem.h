#pragma onc
#include <dpp/dispatcher.h>

namespace LoggingSystem
{
    dpp::task<void> on_automod_rule_create(dpp::cluster* cluster, const dpp::automod_rule_create_t& event);
    dpp::task<void> on_automod_rule_delete(dpp::cluster* cluster, const dpp::automod_rule_delete_t& event);
    dpp::task<void> on_automod_rule_update(dpp::cluster* cluster, const dpp::automod_rule_update_t& event);
    dpp::task<void> on_channel_create(dpp::cluster* cluster, const dpp::channel_create_t& event);
    dpp::task<void> on_channel_delete(dpp::cluster* cluster, const dpp::channel_delete_t& event);
    dpp::task<void> on_channel_update(dpp::cluster* cluster, const dpp::channel_update_t& event);
    dpp::task<void> on_guild_ban_add(dpp::cluster* cluster, const dpp::guild_ban_add_t& event);
    dpp::task<void> on_guild_ban_remove(dpp::cluster* cluster, const dpp::guild_ban_remove_t& event);
    dpp::task<void> on_guild_member_add(dpp::cluster* cluster, const dpp::guild_member_add_t& event);
    dpp::task<void> on_guild_member_remove(dpp::cluster* cluster, const dpp::guild_member_remove_t& event);
    dpp::task<void> on_guild_member_update(dpp::cluster* cluster, const dpp::guild_member_update_t& event);
    dpp::task<void> on_guild_role_create(dpp::cluster* cluster, const dpp::guild_role_create_t& event);
    dpp::task<void> on_guild_role_delete(dpp::cluster* cluster, const dpp::guild_role_delete_t& event);
    dpp::task<void> on_guild_role_update(dpp::cluster* cluster, const dpp::guild_role_update_t& event);
    dpp::task<void> on_guild_scheduled_event_cancel(dpp::cluster* cluster,
        const dpp::guild_scheduled_event_delete_t& event);
    dpp::task<void> on_guild_scheduled_event_complete(dpp::cluster* cluster,
        const dpp::guild* guild, const dpp::scheduled_event& completed);
    dpp::task<void> on_guild_scheduled_event_create(dpp::cluster* cluster,
        const dpp::guild_scheduled_event_create_t& event);
    dpp::task<void> on_guild_scheduled_event_start(dpp::cluster* cluster,
        const dpp::guild* guild, const dpp::scheduled_event& started);
    dpp::task<void> on_guild_scheduled_event_update(dpp::cluster* cluster, const dpp::guild* guild,
        const dpp::scheduled_event& before, const dpp::scheduled_event& after);
    dpp::task<void> on_guild_sticker_create(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker);
    dpp::task<void> on_guild_sticker_delete(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker);
    dpp::task<void> on_guild_sticker_update(dpp::cluster* cluster, const dpp::guild* guild, const dpp::sticker& sticker);
    dpp::task<void> on_guild_update(dpp::cluster* cluster, const dpp::guild_update_t& event);
    dpp::task<void> on_invite_create(dpp::cluster* cluster, const dpp::invite_create_t& event);
    dpp::task<void> on_invite_delete(dpp::cluster* cluster, const dpp::invite_delete_t& event);
    dpp::task<void> on_message_delete(dpp::cluster* cluster, const dpp::message_delete_t& event);
    dpp::task<void> on_message_delete_bulk(dpp::cluster* cluster, const dpp::guild* guild,
                                           const std::vector<dpp::message*>& messages);
    dpp::task<void> on_message_update(dpp::cluster* cluster, const dpp::message_update_t& event);
    dpp::task<void> on_thread_create(dpp::cluster* cluster, const dpp::thread_create_t& event);
    dpp::task<void> on_thread_delete(dpp::cluster* cluster, const dpp::thread_delete_t& event);
    dpp::task<void> on_thread_update(dpp::cluster* cluster, const dpp::thread_update_t& event);
}
