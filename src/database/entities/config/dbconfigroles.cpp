#include "dbconfigroles.h"
#include <bsoncxx/builder/stream/document.hpp>

DbConfigRoles::DbConfigRoles(const bsoncxx::document::value& doc)
{
    djRole = doc["djRole"].get_int64();
    guildId = doc["guildId"].get_int64();
    staffLvl1Role = doc["staffLvl1Role"].get_int64();
    staffLvl2Role = doc["staffLvl2Role"].get_int64();
}

bsoncxx::document::value DbConfigRoles::toDocument() const
{
    return bsoncxx::builder::stream::document()
           << "djRole" << djRole
           << "guildId" << guildId
           << "staffLvl1Role" << staffLvl1Role
           << "staffLvl2Role" << staffLvl2Role
           << bsoncxx::builder::stream::finalize;
}
