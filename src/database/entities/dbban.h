#pragma once
#include "dbobject.h"

struct DbBan : DbObject
{
    int64_t guildId{};
    int64_t time{};
    int64_t userId{};

    DbBan() = default;
    explicit DbBan(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
