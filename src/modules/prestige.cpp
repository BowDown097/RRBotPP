#include "prestige.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigranks.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "dpp-interactive/interactiveservice.h"
#include "utils/ld.h"
#include <dpp/colors.h>
#include <dpp/dispatcher.h>

Prestige::Prestige() : dpp::module<Prestige>("Prestige", "All prestige-related stuffs.")
{
    register_command(&Prestige::doPrestige, "prestige", "Prestige!\n\nUpon prestige, you will **GET**:\n- +1.2x cash multiplier\n- +1.5x rank costs\n- A shiny, cool new badge on $prestigeinfo\n\nand you will **LOSE**:\n- All money, including in crypto investments\n- All cooldowns\n- All items");
    register_command(&Prestige::prestigeInfo, "prestigeinfo", "Check out the perks you're getting from your prestige, and other info.");
}

dpp::task<dpp::command_result> Prestige::doPrestige()
{
    DbConfigRanks ranks = MongoManager::fetchRankConfig(context->msg.guild_id);
    if (ranks.ids.empty())
        co_return dpp::command_result::from_error(Responses::NoRanksConfigured);

    std::vector<std::pair<int, long double>> rankCosts(
        std::make_move_iterator(ranks.costs.begin()),
        std::make_move_iterator(ranks.costs.end()));
    std::ranges::sort(rankCosts, [](const auto& a, const auto& b) { return a.second < b.second; });

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (long double prestigeCost = rankCosts.back().second * (1 + 0.5L * user.prestige); user.cash < prestigeCost)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "have", RR::utility::cash2str(prestigeCost)));
    if (user.prestige >= Constants::MaxPrestige)
        co_return dpp::command_result::from_error(std::format(Responses::MaxPrestigeReached, Constants::MaxPrestige));

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    context->reply(std::format(Responses::PrestigePrompt, Constants::PrestigeTimeout));
    dpp::interactive_service* interactive = extra_data<dpp::interactive_service*>();
    dpp::interactive_result<dpp::message> result = co_await interactive->next_message([this](const dpp::message& m) {
        return m.channel_id == context->msg.channel_id && m.author.id == context->msg.author.id;
    }, std::chrono::seconds(Constants::PrestigeTimeout));

    if (!result.success() || !result.value || !dpp::utility::iequals(result.value->content, "yes"))
        co_return dpp::command_result::from_error(Responses::PrestigeCancelled);

    user.btc = user.eth = user.ltc = user.xrp = 0;
    user.prestige++;
    user.ammo.clear();
    user.collectibles.clear();
    user.consumables.clear();
    user.crates.clear();
    user.perks.clear();
    user.tools.clear();
    user.usedConsumables.clear();
    user.weapons.clear();

    user.messageCashCooldown = 0;
    for (auto& [_, cooldown] : user.constructCooldownMap())
        cooldown = 0;
    for (auto& [_, time] : user.constructEndTimeMap())
        time = 0;

    co_await user.setCashWithoutAdjustment(gm.value(), 0, cluster);
    user.modCooldown(user.prestigeCooldown = Constants::PrestigeCooldown, gm.value());
    user.unlockAchievement("Prestiged!", context);
    if (user.prestige >= Constants::MaxPrestige)
        user.unlockAchievement("Maxed!", context);

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::PrestigeSuccess, user.prestige));
}

dpp::command_result Prestige::prestigeInfo()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.prestige < 1)
        return dpp::command_result::from_error(Responses::NoPrestige);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_description("**Prestige Perks**")
        .set_thumbnail(Constants::PrestigeImages[user.prestige - 1])
        .add_field("Prestige Level", dpp::utility::lexical_cast<std::string>(user.prestige))
        .add_field("Cash Multiplier", std::format("{:.1f}", 1 + 0.2L * user.prestige) + 'x')
        .add_field("Rank Cost Multiplier", std::format("{:.1f}", 1 + 0.5L * user.prestige) + 'x');

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}
