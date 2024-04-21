#include "dbchill.h"
#include <bsoncxx/builder/stream/document.hpp>

DbChill::DbChill(const bsoncxx::document::value& doc)
{
    channelId = doc["channelId"].get_int64();
    guildId = doc["guildId"].get_int64();
    time = doc["time"].get_int64();
}

bsoncxx::document::value DbChill::toDocument() const
{
    return bsoncxx::builder::stream::document()
           << "channelId" << channelId
           << "guildId" << guildId
           << "time" << time
           << bsoncxx::builder::stream::finalize;
}
