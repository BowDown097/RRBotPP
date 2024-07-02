#include "itemsystem.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "dpp-command-handler/utils/strings.h"
#include "entities/goods/crate.h"
#include "entities/goods/perk.h"
#include "entities/goods/tool.h"
#include "utils/ld.h"
#include "utils/timestamp.h"
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <ranges>

#ifndef __cpp_lib_ranges_concat
#include "utils/views_concat/concat.hpp"
#endif

namespace ItemSystem
{
    dpp::task<dpp::command_result> buyCrate(const Crate& crate, const dpp::guild_member& member, DbUser& dbUser,
                                            dpp::cluster* cluster, bool notify)
    {
        std::string crateName(crate.name());
        if (crate.price() > dbUser.cash)
            co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));
        if (dbUser.crates[crateName] >= 10)
            co_return dpp::command_result::from_error(std::format(Responses::ReachedMaxCrates, crate.name()));

        dbUser.crates[crateName]++;
        co_await dbUser.setCashWithoutAdjustment(member, dbUser.cash - crate.price(), cluster);

        if (notify)
            co_return dpp::command_result::from_success(std::format(Responses::BoughtCrate, crate.name(), RR::utility::curr2str(crate.price())));
        co_return dpp::command_result::from_success();
    }

    dpp::task<dpp::command_result> buyPerk(const Perk& perk, const dpp::guild_member& member,
                                           DbUser& dbUser, dpp::cluster* cluster)
    {
        if (perk.price() > dbUser.cash)
            co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));
        if (dbUser.perks.contains(std::string(perk.name())))
            co_return dpp::command_result::from_error(std::format(Responses::AlreadyHaveThing, perk.name()));
        if (dbUser.perks.contains("Pacifist"))
            co_return dpp::command_result::from_error(Responses::HavePacifistPerk);
        if (!dbUser.perks.contains("Multiperk") && dbUser.perks.size() == 1 && perk.name() != "Pacifist" && perk.name() != "Multiperk")
            co_return dpp::command_result::from_error(std::format(Responses::AlreadyHaveAThing, "perk"));
        if (dbUser.perks.contains("Multiperk") && dbUser.perks.size() == 3 && perk.name() != "Pacifist" && perk.name() != "Multiperk")
            co_return dpp::command_result::from_error(std::format(Responses::AlreadyHaveThing, "2 perks"));

        if (perk.name() == "Pacifist")
        {
            if (dbUser.pacifistCooldown > 0)
            {
                if (long cooldownSecs = dbUser.pacifistCooldown - RR::utility::unixTimestamp())
                    co_return dpp::command_result::from_error(std::format(Responses::BoughtPacifistRecently, RR::utility::formatSeconds(cooldownSecs)));
                dbUser.pacifistCooldown = 0;
            }

            for (auto it = dbUser.perks.cbegin(); it != dbUser.perks.cend();)
            {
                if (const Perk* perk = dynamic_cast<const Perk*>(getItem(it->first)))
                    dbUser.cash += perk->price();
                it = dbUser.perks.erase(it);
            }
        }
        else if (perk.name() == "Speed Demon")
        {
            for (const auto& [_, value] : dbUser.constructCooldownMap())
                if (int64_t cooldownSecs = value - RR::utility::unixTimestamp(); cooldownSecs > 0)
                    value = RR::utility::unixTimestamp(cooldownSecs * 0.85);
        }

        dbUser.perks.emplace(perk.name(), RR::utility::unixTimestamp(perk.duration()));
        co_await dbUser.setCashWithoutAdjustment(member, dbUser.cash - perk.price(), cluster);

        co_return dpp::command_result::from_success(perk.name() == "Pacifist"
            ? std::format(Responses::BoughtPacifistPerk, RR::utility::curr2str(perk.price()))
            : std::format(Responses::BoughtPerk, perk.name(), RR::utility::curr2str(perk.price())));
    }

    dpp::task<dpp::command_result> buyTool(const Tool& tool, const dpp::guild_member& member,
                                           DbUser& dbUser, dpp::cluster* cluster)
    {
        if (tool.price() > dbUser.cash)
            co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));
        if (std::ranges::contains(dbUser.tools, tool.name()))
            co_return dpp::command_result::from_error(std::format(Responses::AlreadyHaveAThing, tool.name()));

        dbUser.tools.emplace_back(tool.name());
        co_await dbUser.setCashWithoutAdjustment(member, dbUser.cash - tool.price(), cluster);
        co_return dpp::command_result::from_success(std::format(Responses::BoughtTool, tool.name(), RR::utility::curr2str(tool.price())));
    }

    std::string getBestTool(std::span<const std::string> tools, std::string_view type)
    {
        auto toolsOfType = tools | std::views::filter([type](const std::string& t) { return t.ends_with(type); });
        auto it = std::ranges::max_element(toolsOfType, std::ranges::less(), [](const std::string& t) {
            if (const Item* i = getItem(t))
                return i->price();
            return 0.0L;
        });
        return it != std::ranges::end(toolsOfType) ? *it : std::string();
    }

    const Item* getItem(std::string_view name)
    {
        #define ITEMS_CASTED(arr) \
            arr | std::views::transform([](auto& item) { return dynamic_cast<const Item*>(&item); })

        constexpr auto allItems = std::views::concat(
            ITEMS_CASTED(Constants::Ammos),
            ITEMS_CASTED(Constants::Crates),
            ITEMS_CASTED(Constants::Collectibles),
            ITEMS_CASTED(Constants::Consumables),
            ITEMS_CASTED(Constants::Perks),
            ITEMS_CASTED(Constants::Tools),
            ITEMS_CASTED(Constants::Weapons)
        );

        auto it = std::ranges::find_if(allItems, [name](const Item* i) { return dpp::utility::iequals(i->name(), name); });
        if (it != allItems.end())
            return *it;
        else
            return nullptr;
    }

    void giveCollectible(std::string_view name, const dpp::message_create_t* context, DbUser& dbUser)
    {
        if (const Collectible* collectible = dynamic_cast<const Collectible*>(getItem(name)))
        {
            std::string worthDescription = collectible->price() > 0
                ? RR::utility::curr2str(collectible->price())
                : "some amount of money";

            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::red)
                .set_thumbnail(std::string(collectible->image()))
                .set_title(Responses::CollectibleFoundTitle)
                .set_description(std::format(Responses::CollectibleFoundDescription,
                    collectible->name(), collectible->description(), worthDescription));

            dbUser.collectibles[std::string(collectible->name())]++;
            context->reply(dpp::message(context->msg.channel_id, embed));
        }
    }
}
