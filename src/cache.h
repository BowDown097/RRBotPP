#pragma once
#include "entities/sizedcache.h"

namespace dpp
{
    class automod_rule;
    class message;
    class scheduled_event;
    class stage_instance;
    class sticker;
    class thread;
}

namespace RR
{
    dpp::automod_rule* find_automod_rule(dpp::snowflake id);
    dpp::cache<dpp::automod_rule>* get_automod_rule_cache();

    dpp::message* find_message(dpp::snowflake id);
    sized_cache<dpp::message, 1500>* get_message_cache();

    dpp::scheduled_event* find_scheduled_event(dpp::snowflake id);
    dpp::cache<dpp::scheduled_event>* get_scheduled_event_cache();

    dpp::stage_instance* find_stage(dpp::snowflake id);
    dpp::cache<dpp::stage_instance>* get_stage_cache();

    dpp::sticker* find_sticker(dpp::snowflake id);
    dpp::cache<dpp::sticker>* get_sticker_cache();

    dpp::thread* find_thread(dpp::snowflake id);
    dpp::cache<dpp::thread>* get_thread_cache();
}
