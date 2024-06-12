#include "dbgang.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbGang::DbGang(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    isPublic = bsoncxx_get_or_default(doc["isPublic"], bool);
    leader = bsoncxx_get_or_default(doc["leader"], int64);
    name = bsoncxx_get_value_or_default(doc["name"], string);
    vaultBalance = RR::utility::get_long_double(doc["vaultBalance"]);
    vaultUnlocked = bsoncxx_get_or_default(doc["vaultUnlocked"], bool);
    bsoncxx_elem_to_map(doc["members"], members, "id", int64, "position", int32);
}

bsoncxx::document::value DbGang::toDocument() const
{
    bsoncxx::builder::stream::array membersArr;
    bsoncxx_stream_map_into(members, membersArr, "id", "position");

    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "isPublic" << isPublic
           << "leader" << leader
           << "members" << membersArr
           << "name" << name
           << "vaultBalance" << RR::utility::put_long_double(vaultBalance)
           << "vaultUnlocked" << vaultUnlocked
           << bsoncxx::builder::stream::finalize;
}
