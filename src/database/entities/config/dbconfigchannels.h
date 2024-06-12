#pragma once
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
    explicit DbConfigChannels(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
