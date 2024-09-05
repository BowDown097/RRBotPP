#include "sizedcache.h"

RR::sized_cache<dpp::message, 1500>* message_cache{};

dpp::message* find_message(dpp::snowflake id)
{
    return message_cache ? message_cache->find(id) : nullptr;
}

RR::sized_cache<dpp::message, 1500>* get_message_cache()
{
    if (!message_cache)
        message_cache = new RR::sized_cache<dpp::message, 1500>;
    return message_cache;
}
