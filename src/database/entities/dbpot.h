#pragma once
#include "dbobject.h"
#include <unordered_map>

struct DbPot : DbObject
{
    int64_t endTime = -1;
    int64_t guildId{};
    std::unordered_map<int64_t, long double> members; // user ID, bet
    long double value{};

    DbPot() = default;
    explicit DbPot(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;

    uint64_t drawMember();
    long double getMemberOdds(uint64_t userId);
};
