#include "dbconfigchannels.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigChannels::DbConfigChannels(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    logsChannel = bsoncxx_get_or_default(doc["logsChannel"], int64);
    potChannel = bsoncxx_get_or_default(doc["potChannel"], int64);
    bsoncxx_elem_to_set(doc["noFilterChannels"], noFilterChannels, int64);
    bsoncxx_elem_to_set(doc["whitelistedChannels"], whitelistedChannels, int64);
}

bsoncxx::document::value DbConfigChannels::toDocument() const
{
    bsoncxx::builder::stream::array noFilterChannelsArr;
    bsoncxx_stream_array_into(noFilterChannels, noFilterChannelsArr);

    bsoncxx::builder::stream::array whitelistedChannelsArr;
    bsoncxx_stream_array_into(whitelistedChannels, whitelistedChannelsArr);

    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "logsChannel" << logsChannel
           << "noFilterChannels" << noFilterChannelsArr
           << "potChannel" << potChannel
           << "whitelistedChannels" << whitelistedChannelsArr
           << bsoncxx::builder::stream::finalize;
}
