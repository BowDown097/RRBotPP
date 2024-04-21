#ifndef DBCONFIGCHANNELS_H
#define DBCONFIGCHANNELS_H
#include "database/entities/dbobject.h"
#include <vector>

struct DbConfigChannels : DbObject
{
    int64_t guildId{};
    int64_t logsChannel{};
    std::vector<int64_t> noFilterChannels;
    int64_t potChannel{};
    std::vector<int64_t> whitelistedChannels;

    DbConfigChannels() = default;
    explicit DbConfigChannels(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};

#endif // DBCONFIGCHANNELS_H
