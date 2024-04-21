#include "dbban.h"
#include <bsoncxx/builder/stream/document.hpp>

DbBan::DbBan(const bsoncxx::document::value& doc)
{
    guildId = doc["guildId"].get_int64();
    time = doc["time"].get_int64();
    userId = doc["userId"].get_int64();
}

bsoncxx::document::value DbBan::toDocument() const
{
    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "time" << time
           << "userId" << userId
           << bsoncxx::builder::stream::finalize;
}
