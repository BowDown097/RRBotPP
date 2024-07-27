#pragma once
#include "database/entities/dbobject.h"
#include <set>

struct DbConfigChannels : DbObject
{
    int64_t guildId{};
    int64_t logsChannel{};
    std::set<int64_t> noFilterChannels;
    int64_t potChannel{};
    std::set<int64_t> whitelistedChannels;

    DbConfigChannels() = default;
    explicit DbConfigChannels(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
