#include "dbgang.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/document.hpp>

DbGang::DbGang(const bsoncxx::document::value& doc)
{
    guildId = doc["guildId"].get_int64();
    isPublic = doc["isPublic"].get_bool();
    leader = doc["leader"].get_int64();
    name = doc["name"].get_string();
    vaultBalance = RR::utility::get_long_double(doc["vaultBalance"]);
    vaultUnlocked = doc["vaultUnlocked"].get_bool();

    bsoncxx::document::view membersDoc = doc["members"].get_document();
    for (auto it = membersDoc.cbegin(); it != membersDoc.cend(); ++it)
        members.emplace(dpp::utility::lexical_cast<int64_t>(it->key()), it->get_string());
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
