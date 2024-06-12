#pragma once
#include "database/entities/dbobject.h"
#include <vector>

struct DbConfigGlobal : DbObject
{
    std::vector<int64_t> bannedUsers;
    std::vector<std::string> disabledCommands;

    DbConfigGlobal() = default;
    explicit DbConfigGlobal(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
