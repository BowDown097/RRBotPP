#ifndef DBCHILL_H
#define DBCHILL_H
#include "dbobject.h"

struct DbChill : DbObject
{
    int64_t channelId{};
    int64_t guildId{};
    int64_t time = -1;

    DbChill() = default;
    explicit DbChill(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};

#endif // DBCHILL_H
