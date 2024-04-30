#pragma once
#include "dbobject.h"

struct DbBan : DbObject
{
    int64_t guildId{};
    int64_t time = -1;
    int64_t userId{};

    DbBan() = default;
    explicit DbBan(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};
