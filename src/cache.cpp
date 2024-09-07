#include "cache.h"

namespace RR
{
    sized_cache<dpp::message, 1500>* message_cache{};
    dpp::cache<dpp::sticker>* sticker_cache{};

    dpp::message* find_message(dpp::snowflake id)
    {
        return message_cache ? message_cache->find(id) : nullptr;
    }

    sized_cache<dpp::message, 1500>* get_message_cache()
    {
        if (!message_cache)
            message_cache = new sized_cache<dpp::message, 1500>;
        return message_cache;
    }

    dpp::sticker* find_sticker(dpp::snowflake id)
    {
        return sticker_cache ? sticker_cache->find(id) : nullptr;
    }

    dpp::cache<dpp::sticker>* get_sticker_cache()
    {
        if (!sticker_cache)
            sticker_cache = new dpp::cache<dpp::sticker>;
        return sticker_cache;
    }
}
