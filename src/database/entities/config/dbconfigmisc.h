#ifndef DBCONFIGMISC_H
#define DBCONFIGMISC_H
#include "database/entities/dbobject.h"
#include <vector>

struct DbConfigMisc : DbObject
{
    std::vector<std::string> disabledCommands;
    std::vector<std::string> disabledModules;
    bool dropsDisabled{};
    int64_t guildId{};
    bool inviteFilterEnabled{};
    bool nsfwEnabled{};
    bool scamFilterEnabled{};

    DbConfigMisc() = default;
    explicit DbConfigMisc(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};

#endif // DBCONFIGMISC_H
