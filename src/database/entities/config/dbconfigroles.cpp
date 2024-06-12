#include "dbconfigroles.h"
#include <bsoncxx/builder/stream/document.hpp>

DbConfigRoles::DbConfigRoles(bsoncxx::document::view doc)
{
    djRole = bsoncxx_get_or_default(doc["djRole"], int64);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    staffLvl1Role = bsoncxx_get_or_default(doc["staffLvl1Role"], int64);
    staffLvl2Role = bsoncxx_get_or_default(doc["staffLvl2Role"], int64);
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
