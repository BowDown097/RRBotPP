#include "dbconfigranks.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigRanks::DbConfigRanks(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);

    bsoncxx::array::view costsArr = bsoncxx_get_or_default(doc["costs"], array);
    for (auto it = costsArr.cbegin(); it != costsArr.cend(); ++it)
    {
        bsoncxx::document::view costDoc = bsoncxx_get_or_default((*it), document);
        costs.emplace(bsoncxx_get_or_default(costDoc["level"], int32), RR::utility::get_long_double(costDoc["cost"]));
    }

    bsoncxx_elem_to_map(doc["ids"], ids, "level", int32, "id", int64);
}

bsoncxx::document::value DbConfigRanks::toDocument() const
{
    bsoncxx::builder::stream::array costsArr;
    for (const auto& [level, cost] : costs)
    {
        costsArr << bsoncxx::builder::stream::open_document
                 << "level" << level
                 << "cost" << RR::utility::put_long_double(cost)
                 << bsoncxx::builder::stream::close_document;
    }

    bsoncxx::builder::stream::array idsArr;
    bsoncxx_stream_map_into(ids, idsArr, "level", "id");

    return bsoncxx::builder::stream::document()
           << "costs" << costsArr
           << "guildId" << guildId
           << "ids" << idsArr
           << bsoncxx::builder::stream::finalize;
}
