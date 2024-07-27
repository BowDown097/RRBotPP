#pragma once
#include "database/entities/dbobject.h"
#include <set>

struct DbConfigGlobal : DbObject
{
    std::set<int64_t> bannedUsers;
    std::set<std::string> disabledCommands;

    DbConfigGlobal() = default;
    explicit DbConfigGlobal(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
