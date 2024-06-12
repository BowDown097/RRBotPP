#include "dbban.h"
#include <bsoncxx/builder/stream/document.hpp>

DbBan::DbBan(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    time = bsoncxx_get_or_default(doc["time"], int64);
    userId = bsoncxx_get_or_default(doc["userId"], int64);
}

bsoncxx::document::value DbBan::toDocument() const
{
    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "time" << time
           << "userId" << userId
           << bsoncxx::builder::stream::finalize;
}
