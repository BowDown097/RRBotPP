#pragma once
#include "database/entities/dbobject.h"

struct DbConfigRoles : DbObject
{
    int64_t djRole{};
    int64_t guildId{};
    int64_t staffLvl1Role{};
    int64_t staffLvl2Role{};

    DbConfigRoles() = default;
    explicit DbConfigRoles(const bsoncxx::document::value& doc);
    bsoncxx::document::value toDocument() const override;
};
