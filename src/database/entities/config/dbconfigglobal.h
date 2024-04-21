#ifndef DBGLOBALCONFIG_H
#define DBGLOBALCONFIG_H
#include "database/entities/dbobject.h"
#include <vector>

struct DbConfigGlobal : DbObject
{
    std::vector<int64_t> bannedUsers;
    std::vector<std::string> disabledCommands;

    DbConfigGlobal() = default;
    explicit DbConfigGlobal(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};

#endif // DBGLOBALCONFIG_H
