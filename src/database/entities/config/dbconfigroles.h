#pragma once
#include "database/entities/dbobject.h"

namespace dpp { class guild_member; }

struct DbConfigRoles : DbObject
{
    int64_t djRole{};
    int64_t guildId{};
    int64_t staffLvl1Role{};
    int64_t staffLvl2Role{};

    DbConfigRoles() = default;
    explicit DbConfigRoles(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;

    bool memberIsStaff(const dpp::guild_member& member) const;
};
