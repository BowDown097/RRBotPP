#pragma once
#include "dbobject.h"

struct DbChill : DbObject
{
    int64_t channelId{};
    int64_t guildId{};
    int64_t time{};

    DbChill() = default;
    explicit DbChill(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
