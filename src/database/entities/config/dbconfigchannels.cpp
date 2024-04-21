#include "dbconfigchannels.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigChannels::DbConfigChannels(const bsoncxx::document::value& doc)
{
    guildId = doc["guildId"].get_int64();
    logsChannel = doc["logsChannel"].get_int64();
    potChannel = doc["potChannel"].get_int64();

    bsoncxx::array::view noFilterChannelsArr = doc["noFilterChannels"].get_array();
    for (auto it = noFilterChannelsArr.cbegin(); it != noFilterChannelsArr.cend(); ++it)
        noFilterChannels.push_back(it->get_int64());

    bsoncxx::array::view whitelistedChannelsArr = doc["whitelistedChannels"].get_array();
    for (auto it = whitelistedChannelsArr.cbegin(); it != whitelistedChannelsArr.cend(); ++it)
        whitelistedChannels.push_back(it->get_int64());
}

bsoncxx::document::value DbConfigChannels::toDocument() const
{
    bsoncxx::builder::stream::array noFilterChannelsArr;
    for (int64_t noFilterChannel : noFilterChannels)
        noFilterChannelsArr << noFilterChannel;

    bsoncxx::builder::stream::array whitelistedChannelsArr;
    for (int64_t whitelistedChannel : whitelistedChannels)
        whitelistedChannelsArr << whitelistedChannel;

    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "logsChannel" << logsChannel
           << "noFilterChannels" << noFilterChannelsArr
           << "potChannel" << potChannel
           << "whitelistedChannels" << whitelistedChannelsArr
           << bsoncxx::builder::stream::finalize;
}
