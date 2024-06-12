#include "dbpot.h"
#include "utils/ld.h"
#include "utils/random.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbPot::DbPot(bsoncxx::document::view doc)
{
    endTime = bsoncxx_get_or_default(doc["endTime"], int64);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    value = RR::utility::get_long_double(doc["value"]);

    bsoncxx::array::view membersArr = bsoncxx_get_or_default(doc["members"], array);
    for (auto it = membersArr.cbegin(); it != membersArr.cend(); ++it)
    {
        bsoncxx::document::view memberDoc = bsoncxx_get_or_default((*it), document);
        members.emplace(bsoncxx_get_or_default(memberDoc["id"], int64), RR::utility::get_long_double(memberDoc["bet"]));
    }
}

bsoncxx::document::value DbPot::toDocument() const
{
    bsoncxx::builder::stream::array membersArr;
    for (const auto& [id, bet] : members)
    {
        membersArr << bsoncxx::builder::stream::open_document
                   << "id" << id
                   << "bet" << RR::utility::put_long_double(bet)
                   << bsoncxx::builder::stream::close_document;
    }

    return bsoncxx::builder::stream::document()
           << "endTime" << endTime
           << "guildId" << guildId
           << "members" << membersArr
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
