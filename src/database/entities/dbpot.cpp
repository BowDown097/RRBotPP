#include "dbpot.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "utils/ld.h"
#include "utils/random.h"
#include <bsoncxx/builder/stream/document.hpp>

DbPot::DbPot(bsoncxx::document::view doc)
{
    endTime = bsoncxx_get_or_default(doc["endTime"], int64);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    value = RR::utility::get_long_double(doc["value"]);

    bsoncxx::document::view membersDoc = bsoncxx_get_or_default(doc["members"], document);
    for (auto it = membersDoc.cbegin(); it != membersDoc.cend(); ++it)
        members.emplace(dpp::utility::lexical_cast<int64_t>(it->key()), RR::utility::get_long_double(*it));
}

bsoncxx::document::value DbPot::toDocument() const
{
    bsoncxx::builder::stream::document membersDoc;
    for (const auto& [userId, bet] : members)
        membersDoc << dpp::utility::lexical_cast<std::string>(userId) << RR::utility::put_long_double(bet);

    return bsoncxx::builder::stream::document()
           << "endTime" << endTime
           << "guildId" << guildId
           << "members" << membersDoc
           << "value" << RR::utility::put_long_double(value)
           << bsoncxx::builder::stream::finalize;
}

uint64_t DbPot::drawMember()
{
    double ranges[2] = {0};
    int roll = RR::utility::random(100);
    for (const auto& [userId, _] : members)
    {
        long double odds = getMemberOdds(userId);
        std::swap(ranges[0], ranges[1]);
        ranges[1] = ranges[0] + odds;
        if (roll > ranges[0] && roll <= ranges[1])
            return userId;
    }

    return 0;
}

long double DbPot::getMemberOdds(uint64_t userId)
{
    if (auto it = members.find(userId); it != members.end())
        return it->second / value * 100;
    return 0;
}
