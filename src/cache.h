#pragma once
#include "entities/sizedcache.h"

namespace dpp { class message; class sticker; }

namespace RR
{
    dpp::message* find_message(dpp::snowflake id);
    sized_cache<dpp::message, 1500>* get_message_cache();

    dpp::sticker* find_sticker(dpp::snowflake id);
    dpp::cache<dpp::sticker>* get_sticker_cache();
}
