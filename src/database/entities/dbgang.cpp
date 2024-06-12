#include "dbgang.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/document.hpp>

DbGang::DbGang(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    isPublic = bsoncxx_get_or_default(doc["isPublic"], bool);
    leader = bsoncxx_get_or_default(doc["leader"], int64);
    name = bsoncxx_get_value_or_default(doc["name"], string);
    vaultBalance = RR::utility::get_long_double(doc["vaultBalance"]);
    vaultUnlocked = bsoncxx_get_or_default(doc["vaultUnlocked"], bool);

    bsoncxx::document::view membersDoc = bsoncxx_get_or_default(doc["members"], document);
    for (auto it = membersDoc.cbegin(); it != membersDoc.cend(); ++it)
        members.emplace(dpp::utility::lexical_cast<int64_t>(it->key()), it->get_int32());
}

bsoncxx::document::value DbGang::toDocument() const
{
    bsoncxx::builder::stream::document membersDoc;
    for (const auto& [userId, position] : members)
        membersDoc << dpp::utility::lexical_cast<std::string>(userId) << position;

    return bsoncxx::builder::stream::document()
           << "guildId" << guildId
           << "isPublic" << isPublic
           << "leader" << leader
           << "members" << membersDoc
           << "name" << name
           << "vaultBalance" << RR::utility::put_long_double(vaultBalance)
           << "vaultUnlocked" << vaultUnlocked
           << bsoncxx::builder::stream::finalize;
}
