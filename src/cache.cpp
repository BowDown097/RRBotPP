#include "cache.h"

namespace RR
{
    dpp::cache<dpp::automod_rule>* automod_rule_cache{};
    sized_cache<dpp::message, 1500>* message_cache{};
    dpp::cache<dpp::scheduled_event>* scheduled_event_cache{};
    dpp::cache<dpp::stage_instance>* stage_cache{};
    dpp::cache<dpp::sticker>* sticker_cache{};
    dpp::cache<dpp::thread>* thread_cache{};

    dpp::automod_rule* find_automod_rule(dpp::snowflake id)
    {
        return get_automod_rule_cache()->find(id);
    }

    dpp::message* find_message(dpp::snowflake id)
    {
        return get_message_cache()->find(id);
    }

    dpp::scheduled_event* find_scheduled_event(dpp::snowflake id)
    {
        return get_scheduled_event_cache()->find(id);
    }

    dpp::stage_instance* find_stage(dpp::snowflake id)
    {
        return get_stage_cache()->find(id);
    }

    dpp::sticker* find_sticker(dpp::snowflake id)
    {
        return get_sticker_cache()->find(id);
    }

    dpp::thread* find_thread(dpp::snowflake id)
    {
        return get_thread_cache()->find(id);
    }

    dpp::cache<dpp::automod_rule>* get_automod_rule_cache()
    {
        if (!automod_rule_cache)
            automod_rule_cache = new dpp::cache<dpp::automod_rule>;
        return automod_rule_cache;
    }

    sized_cache<dpp::message, 1500>* get_message_cache()
    {
        if (!message_cache)
            message_cache = new sized_cache<dpp::message, 1500>;
        return message_cache;
    }

    dpp::cache<dpp::scheduled_event>* get_scheduled_event_cache()
    {
        if (!scheduled_event_cache)
            scheduled_event_cache = new dpp::cache<dpp::scheduled_event>;
        return scheduled_event_cache;
    }

    dpp::cache<dpp::stage_instance>* get_stage_cache()
    {
        if (!stage_cache)
            stage_cache = new dpp::cache<dpp::stage_instance>;
        return stage_cache;
    }

    dpp::cache<dpp::sticker>* get_sticker_cache()
    {
        if (!sticker_cache)
            sticker_cache = new dpp::cache<dpp::sticker>;
        return sticker_cache;
    }

    dpp::cache<dpp::thread>* get_thread_cache()
    {
        if (!thread_cache)
            thread_cache = new dpp::cache<dpp::thread>;
        return thread_cache;
    }
}
