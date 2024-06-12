#include "dbconfigranks.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigRanks::DbConfigRanks(bsoncxx::document::view doc)
{
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);

    bsoncxx::document::view costsDoc = bsoncxx_get_or_default(doc["costs"], document);
    for (auto it = costsDoc.cbegin(); it != costsDoc.cend(); ++it)
        costs.emplace(dpp::utility::lexical_cast<int>(it->key()), RR::utility::get_long_double(*it));

    bsoncxx_elem_to_map(doc["ids"], ids, int64);
}

bsoncxx::document::value DbConfigRanks::toDocument() const
{
    bsoncxx::builder::stream::document costsDoc;
    for (const auto& [level, cost] : costs)
        costsDoc << dpp::utility::lexical_cast<std::string>(level) << RR::utility::put_long_double(cost);

    bsoncxx::builder::stream::document idsDoc;
    bsoncxx_stream_map_into(ids, idsDoc);

    return bsoncxx::builder::stream::document()
           << "costs" << costsDoc
           << "guildId" << guildId
           << "ids" << idsDoc
           << bsoncxx::builder::stream::finalize;
}
