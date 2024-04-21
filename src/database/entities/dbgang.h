#ifndef DBGANG_H
#define DBGANG_H
#include "dbobject.h"
#include <unordered_map>

struct DbGang : DbObject
{
    int64_t guildId{};
    bool isPublic{};
    int64_t leader{};
    std::unordered_map<int64_t, std::string> members; // user ID, position
    std::string name;
    long double vaultBalance{};
    bool vaultUnlocked{};

    DbGang() = default;
    explicit DbGang(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};

#endif // DBGANG_H
