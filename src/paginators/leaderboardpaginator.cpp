#include "leaderboardpaginator.h"
#include "data/constants.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-interactive/pagination/interactionpage.h"
#include "dppcmd/extensions/cache.h"
#include "utils/ld.h"
#include "utils/strings.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <dpp/message.h>
#include <mongocxx/collection.hpp>
#include <mongocxx/options/find.hpp>

dpp::interaction_page LeaderboardPaginator::get_or_load_page(int pageIndex)
{
    dpp::interaction_page page;
    page.set_title(currency == "cash" ? "Leaderboard" : RR::utility::toUpper(currency) + " Leaderboard");

    int start = 10 * pageIndex;

    using namespace bsoncxx::builder;
    mongocxx::options::find findOpts;
    findOpts.collation(stream::document() << "locale" << "en" << "numericOrdering" << true << stream::finalize);
    findOpts.skip(start + skippedUsers);
    findOpts.sort(stream::document() << currency << -1 << stream::finalize);

    mongocxx::cursor cursor = MongoManager::users().find(stream::document() << "guildId" << guildId << stream::finalize, findOpts);
    std::string lb;
    int processedUsers{};

    for (auto it = cursor.begin(); it != cursor.end(); ++it, ++processedUsers)
    {
        if (processedUsers == 10)
            break;

        DbUser dbUser(*it);
        const dpp::user* user = dppcmd::find_guild_user(dbUser.guildId, dbUser.userId);
        if (!user || dbUser.perks.contains("Pacifist"))
        {
            if (pageIndex >= lastPageIndex)
                ++skippedUsers;
            continue;
        }

        long double val = currency == "cash" ? dbUser.cash : *dbUser.getCrypto(currency);
        if (val < Constants::InvestmentMinAmount)
            break;

        int pos = start + 1 + processedUsers;
        std::string mention = user->get_mention();
        lb += currency == "cash"
            ? std::format("{}: {}: {}\n", pos, mention, RR::utility::cash2str(val))
            : std::format("{}: {}: {} ({})\n", pos, mention, RR::utility::roundAsStr(val, 4), RR::utility::cash2str(currencyValue * val));
    }

    // get rid of trailing newline
    if (!lb.empty())
        lb.pop_back();

    lastPageIndex = pageIndex;
    page.set_description(!lb.empty() ? lb : "Nothing to see here!");
    return page;
}
