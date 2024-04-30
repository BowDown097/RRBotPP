#pragma once
#include "database/entities/dbobject.h"
#include <unordered_map>

struct DbConfigRanks : DbObject
{
    std::unordered_map<int, long double> costs; // level, cost
    int64_t guildId{};
    std::unordered_map<int, int64_t> ids; // level, id

    DbConfigRanks() = default;
    explicit DbConfigRanks(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};
