#include "economy.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigranks.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "utils/ld.h"
#include "utils/rrutils.h"
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>

Economy::Economy() : ModuleBase("Economy", "This is the hub for checking and managing your economy stuff. Wanna know how much cash you have? Or what items you have? Or do you want to check out le shoppe? It's all here.")
{
    registerCommand(&Economy::balance, std::initializer_list<std::string> { "balance", "bal", "cash" }, "Check your own or someone else's balance.", "$balance <user>");
    registerCommand(&Economy::cooldowns, std::initializer_list<std::string> { "cooldowns", "cd" }, "Check your own or someone else's command cooldowns.", "$cooldowns <user>");
    registerCommand(&Economy::profile, "profile", "View a bunch of economy-related info on yourself or another user.", "$profile <user>");
    registerCommand(&Economy::ranks, "ranks", "View all the ranks and their costs.");
    registerCommand(&Economy::sauce, std::initializer_list<std::string> { "sauce", "give", "transfer" }, "Sauce someone some cash.");
}

CommandResult Economy::balance(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (dbUser.cash < 0.01L)
    {
        return CommandResult::fromError(user->id == context->msg.author.id
            ? Responses::YouAreBroke : std::format(Responses::UserIsBroke, user->get_mention()));
    }

    return CommandResult::fromSuccess(user->id == context->msg.author.id
        ? std::format(Responses::YourBalance, RR::utility::currencyToStr(dbUser.cash))
        : std::format(Responses::UserBalance, user->get_mention(), RR::utility::currencyToStr(dbUser.cash)));
}

CommandResult Economy::cooldowns(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    std::string description;
    for (const auto& [name, value] : dbUser.constructCooldownMap())
        if (int64_t cooldownSecs = value - RR::utility::unixTimeSecs(); cooldownSecs > 0)
            description += "**" + name + "**: " + RR::utility::formatTimestamp(cooldownSecs) + '\n';
    if (!description.empty())
        description.pop_back();

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Cooldowns")
        .set_description(!description.empty() ? description : "None");

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult Economy::profile(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    std::optional<dpp::guild_member> guildMember = RR::utility::findGuildMember(context->msg.guild_id, user->id);
    if (!guildMember)
        return CommandResult::fromError(Responses::GetUserFailed);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_author(RR::utility::asEmbedAuthor(guildMember.value(), user))
        .set_title("User Profile");

    std::string essentials = "**Cash**: " + RR::utility::currencyToStr(dbUser.cash);
    if (!dbUser.gang.empty())
        essentials += "\n**Gang**: " + dbUser.gang;
    essentials += std::format("\n**Health**: {}", dbUser.health);
    embed.add_field("Essentials", essentials);

    std::string crypto;
    if (dbUser.btc >= 0.01L)
        crypto += std::format("**BTC**: {:.4f}\n", dbUser.btc);
    if (dbUser.eth >= 0.01L)
        crypto += std::format("**ETH**: {:.4f}\n", dbUser.eth);
    if (dbUser.ltc >= 0.01L)
        crypto += std::format("**LTC**: {:.4f}\n", dbUser.ltc);
    if (dbUser.xrp >= 0.01L)
        crypto += std::format("**XRP**: {:.4f}\n", dbUser.xrp);

    if (!crypto.empty())
    {
        crypto.pop_back(); // remove trailing newline
        embed.add_field("Crypto", crypto);
    }

    std::string items;
    if (!dbUser.tools.empty())
        items += std::format("**Tools**: {}\n", dbUser.tools.size());
    if (!dbUser.perks.empty())
        items += std::format("**Perks**: {}\n", dbUser.perks.size());
    if (!dbUser.consumables.empty())
        items += std::format("**Consumables**: {}\n", dbUser.consumables.size());
    if (!dbUser.crates.empty())
        items += std::format("**Crates**: {}\n", dbUser.crates.size());

    if (!items.empty())
    {
        items.pop_back(); // remove trailing newline
        embed.add_field("Items", items);
    }

    std::string activeConsumables;
    for (const auto& [name, uses] : dbUser.usedConsumables)
        if (uses > 0)
            activeConsumables += std::format("**{}**: {}x\n", name, uses);

    if (!activeConsumables.empty())
    {
        activeConsumables.pop_back(); // remove trailing newline
        embed.add_field("Active Consumables", activeConsumables);
    }

    std::string counts = std::format("**Achievements**: {}", dbUser.achievements.size());
    long cooldowns = std::ranges::count_if(dbUser.constructCooldownMap(), [](const std::pair<std::string, int64_t>& p) {
        return p.second - RR::utility::unixTimeSecs() > 0;
    });
    counts += std::format("\n**Commands On Cooldown**: {}", cooldowns);
    embed.add_field("Counts", counts);

    std::string misc;
    if (dbUser.gamblingMultiplier > 1.0L)
        misc += std::format("**Gambling Multiplier**: {:.2f}\n", dbUser.gamblingMultiplier);
    if (dbUser.prestige > 0)
        misc += std::format("**Prestige**: {}\n", dbUser.prestige);

    if (!misc.empty())
    {
        misc.pop_back(); // remove trailing newline
        embed.add_field("Misc", misc);
    }

    context->reply(dpp::message(context->msg.guild_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult Economy::ranks()
{
    DbConfigRanks ranks = MongoManager::fetchRankConfig(context->msg.guild_id);
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);

    std::string description;
    for (const auto& [level, cost] : ranks.costs)
        if (dpp::role* role = dpp::find_role(ranks.ids[level]))
            description += std::format("**{}**: {}", role->name, RR::utility::currencyToStr(cost));

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Available Ranks")
        .set_description(!description.empty() ? description : "None");

    context->reply(dpp::message(context->msg.guild_id, embed));
    return CommandResult::fromSuccess();
}

dpp::task<CommandResult> Economy::sauce(const UserTypeReader& userRead, long double amount)
{
    if (amount < Constants::TransactionMin)
        co_return CommandResult::fromError(std::format(Responses::SauceTooLow, RR::utility::currencyToStr(Constants::TransactionMin)));

    dpp::user* user = userRead.topResult();
    if (user->id == context->msg.author.id)
        co_return CommandResult::fromError(Responses::BadIdea);
    if (user->is_bot())
        co_return CommandResult::fromError(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.cash < amount)
        co_return CommandResult::fromError(Responses::NotEnoughCash);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (target.usingSlots)
        co_return CommandResult::fromError(Responses::UserIsGambling);

    std::optional<dpp::guild_member> authorGM = RR::utility::findGuildMember(context->msg.guild_id, context->msg.author.id);
    if (!authorGM)
        co_return CommandResult::fromError(Responses::GetUserFailed);

    std::optional<dpp::guild_member> targetGM = RR::utility::findGuildMember(context->msg.guild_id, user->id);
    if (!targetGM)
        co_return CommandResult::fromError(Responses::GetUserFailed);

    co_await author.setCash(authorGM.value(), author.cash - amount, cluster, context);
    co_await target.setCash(targetGM.value(), target.cash + amount, cluster, context);

    MongoManager::updateUser(author);
    MongoManager::updateUser(target);
    co_return CommandResult::fromSuccess(std::format(Responses::SaucedUser, user->get_mention(), RR::utility::currencyToStr(amount)));
}
