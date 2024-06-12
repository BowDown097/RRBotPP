#include "dbchill.h"
#include <bsoncxx/builder/stream/document.hpp>

DbChill::DbChill(bsoncxx::document::view doc)
{
    channelId = bsoncxx_get_or_default(doc["channelId"], int64);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    time = bsoncxx_get_or_default(doc["time"], int64);
}

bsoncxx::document::value DbChill::toDocument() const
{
    return bsoncxx::builder::stream::document()
           << "channelId" << channelId
           << "guildId" << guildId
           << "time" << time
           << bsoncxx::builder::stream::finalize;
}
