#include "crime.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigroles.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "utils/ld.h"
#include "utils/random.h"
#include "utils/strings.h"
#include <dpp/cluster.h>

Crime::Crime() : dpp::module_base("Crime", "Hell yeah! Crime! Reject the ways of being a law-abiding citizen for some cold hard cash and maybe even a tool. Or, maybe not. Depends how good you are at being a criminal.")
{
    register_command(&Crime::bully, "bully", "Change the nickname of any victim you wish.", "$bully [user] [nickname]");
    register_command(&Crime::deal, "deal", "Deal some drugs.");
    register_command(&Crime::loot, "loot", "Loot some locations.");
    register_command(&Crime::rape, std::initializer_list<std::string> { "rape", "strugglesnuggle" }, "Get yourself some ass!", "$rape [user]");
    register_command(&Crime::rob, "rob", "Yoink money from a user.", "$rob [user] [amount]");
    register_command(&Crime::slavery, "slavery", "Get some slave labor goin'.");
    register_command(&Crime::whore, "whore", "Sell your body for quick cash.");
}

dpp::task<dpp::command_result> Crime::bully(const dpp::guild_member_in& memberIn,
                                            const dpp::remainder<std::string>& nickname)
{
    if (nickname->size() > 32)
        co_return dpp::command_result::from_error(Responses::NicknameTooLong);

    dpp::guild_member member = memberIn.top_result();
    dpp::user* user = member.get_user();
    if (!user)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->id == context->msg.author.id)
        co_return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (target.perks.contains("Pacifist"))
        co_return dpp::command_result::from_error(std::format(Responses::UserHasPacifist, "bully", user->get_mention()));

    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    const std::vector<dpp::snowflake>& memberRoles = member.get_roles();
    if (std::ranges::contains(memberRoles, dpp::snowflake(roles.staffLvl1Role)) ||
        std::ranges::contains(memberRoles, dpp::snowflake(roles.staffLvl2Role)))
    {
        co_return dpp::command_result::from_error(std::format(Responses::UserIsStaff, "bully", user->get_mention()));
    }

    member.set_nickname(*nickname);
    co_await cluster->co_guild_edit_member(member);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    author.modCooldown(author.bullyCooldown = Constants::BullyCooldown, member);
    MongoManager::updateUser(author);

    co_return dpp::command_result::from_success(std::format(Responses::Bullied, user->get_mention(), RR::utility::sanitizeString(*nickname)));
}

dpp::task<dpp::command_result> Crime::deal()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::DealSuccesses, Responses::DealFails, user,
                                    user.dealCooldown = Constants::DealCooldown, true);
}

dpp::task<dpp::command_result> Crime::loot()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::LootSuccesses, Responses::LootFails, user,
                                    user.lootCooldown = Constants::LootCooldown, true);
}

dpp::task<dpp::command_result> Crime::rape(const dpp::guild_member_in& memberIn)
{
    dpp::guild_member member = memberIn.top_result();
    dpp::user* user = member.get_user();
    if (!user)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->id == context->msg.author.id)
        co_return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dpp::command_result::from_error(std::format(Responses::UserIsGambling, user->get_mention()));
    if (target.perks.contains("Pacifist"))
        co_return dpp::command_result::from_error(std::format(Responses::UserHasPacifist, "rape", user->get_mention()));
    if (target.cash < 0.01L)
        co_return dpp::command_result::from_error(std::format(Responses::UserIsBroke, user->get_mention()));

    std::optional<dpp::guild_member> authorMember = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    double odds = Constants::RapeOdds;
    if (author.usedConsumables.contains("Viagra"))
        odds += 10;
    if (author.perks.contains("Speed Demon"))
        odds *= 0.95;

    long double rapePercent = RR::utility::random(Constants::RapeMinPercent, Constants::RapeMaxPercent);
    if (RR::utility::random(1, 101) < odds)
    {
        long double repairs = target.cash / 100.0L * rapePercent;
        statUpdate(target, false, repairs);
        context->reply(std::format(Responses::RapeSuccess, user->get_mention(), RR::utility::currencyToStr(repairs)));
        co_await target.setCashWithoutAdjustment(member, target.cash - repairs, cluster, context);
    }
    else
    {
        long double repairs = author.cash / 100.0L * rapePercent;
        statUpdate(author, false, repairs);
        context->reply(std::format(Responses::RapeFailed, user->get_mention(), RR::utility::currencyToStr(repairs)));
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash - repairs, cluster, context);
    }

    author.modCooldown(author.rapeCooldown = Constants::RapeCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);

    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Crime::rob(const dpp::guild_member_in& memberIn, const cash_in& amountIn)
{
    long double amount = amountIn.top_result();
    if (amount < Constants::RobMinCash)
        co_return dpp::command_result::from_error(std::format(Responses::RobTooSmall, RR::utility::currencyToStr(Constants::RobMinCash)));

    dpp::guild_member member = memberIn.top_result();
    dpp::user* user = member.get_user();
    if (!user)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->id == context->msg.author.id)
        co_return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dpp::command_result::from_error(std::format(Responses::UserIsGambling, user->get_mention()));
    if (target.perks.contains("Pacifist"))
        co_return dpp::command_result::from_error(std::format(Responses::UserHasPacifist, "rape", user->get_mention()));

    long double robMax = target.cash / 100.0L * Constants::RobMaxPercent;
    if (amount > robMax)
    {
        co_return dpp::command_result::from_error(std::format(Responses::RobTooLarge, Constants::RobMaxPercent,
            user->get_mention(), RR::utility::currencyToStr(robMax)));
    }

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.cash < amount)
        co_return dpp::command_result::from_error(Responses::NotEnoughCash);

    std::optional<dpp::guild_member> authorMember = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    double odds = Constants::RobOdds;
    if (author.usedConsumables.contains("Romanian Flag"))
        odds += 10;
    if (author.perks.contains("Speed Demon"))
        odds *= 0.95;

    if (RR::utility::random(1, 101) < odds)
    {
        co_await target.setCashWithoutAdjustment(member, target.cash - amount, cluster, context);
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash + amount, cluster, context);
        statUpdate(author, true, amount);
        statUpdate(target, false, amount);

        context->reply(std::format("{}\nBalance: {}", RR::utility::randomElement(Responses::RobFails),
                                   RR::utility::currencyToStr(author.cash)));
    }
    else
    {
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash - amount, cluster, context);
        statUpdate(author, false, amount);

        context->reply(std::format("{}\nBalance: {}", RR::utility::randomElement(Responses::RobSuccesses),
                                   RR::utility::currencyToStr(author.cash)));
    }

    author.modCooldown(author.robCooldown = Constants::RobCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);

    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Crime::slavery()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::SlaverySuccesses, Responses::SlaveryFails, user,
                                    user.slaveryCooldown = Constants::SlaveryCooldown);
}

dpp::task<dpp::command_result> Crime::whore()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::WhoreSuccesses, Responses::WhoreFails, user,
                                    user.whoreCooldown = Constants::WhoreCooldown);
}

dpp::task<dpp::command_result> Crime::genericCrime(const std::span<const std::string_view>& successOutcomes,
                                                   const std::span<const std::string_view>& failOutcomes,
                                                   DbUser& user, int64_t& cooldown, bool hasMehOutcome)
{
    std::optional<dpp::guild_member> member = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    double winOdds = user.perks.contains("Speed Demon")
                         ? Constants::GenericCrimeWinOdds * 0.95 : Constants::GenericCrimeWinOdds;

    std::string outcome;
    long double totalCash;
    if (RR::utility::random(1, 101) < winOdds)
    {
        size_t outcomeNum = RR::utility::random(successOutcomes.size());
        long double moneyEarned = RR::utility::random(Constants::GenericCrimeWinMin, Constants::GenericCrimeWinMax);
        if (hasMehOutcome && outcomeNum == successOutcomes.size() - 1)
            moneyEarned /= 5;

        std::string curr = RR::utility::currencyToStr(moneyEarned);
        outcome = std::vformat(successOutcomes[outcomeNum], std::make_format_args(curr));
        totalCash = user.cash + moneyEarned;
        statUpdate(user, true, moneyEarned);
    }
    else
    {
        long double lostCash = RR::utility::random(Constants::GenericCrimeLossMin, Constants::GenericCrimeLossMax);
        if (lostCash > user.cash)
            lostCash = user.cash;

        std::string curr = RR::utility::currencyToStr(lostCash);
        outcome = std::vformat(RR::utility::randomElement(failOutcomes), std::make_format_args(curr));
        totalCash = user.cash - lostCash;
        statUpdate(user, false, lostCash);
    }

    co_await user.setCash(member.value(), totalCash, cluster, context,
                          std::format("{}\nBalance: {}", outcome, RR::utility::currencyToStr(totalCash)));

    if (RR::utility::random(1, 101) < Constants::GenericCrimeToolOdds)
    {
        auto availableTools = Constants::Tools
            | std::views::filter([&user](const Tool& t) { return !std::ranges::contains(user.tools, t.name()); })
            | std::views::transform([](const Tool& t) { return t.name(); });

        if (size_t cnt = std::ranges::distance(availableTools))
        {
            std::string tool(*std::ranges::next(std::ranges::begin(availableTools), RR::utility::random(cnt)));
            user.tools.push_back(tool);
            context->reply(std::format(Responses::GotTool, tool));
        }
    }

    user.modCooldown(cooldown, member.value());
    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success();
}

void Crime::statUpdate(DbUser& user, bool success, long double gain)
{
    std::string gainStr = RR::utility::currencyToStr(gain);
    if (success)
    {
        user.mergeStats(std::unordered_map<std::string, std::string> {
            { "Crimes Succeeded", "1" },
            { "Money Gained from Crimes", gainStr },
            { "Net Gain/Loss from Crimes", gainStr }
        });
    }
    else
    {
        user.mergeStats(std::unordered_map<std::string, std::string> {
            { "Crimes Failed", "1" },
            { "Money Lost to Crimes", gainStr },
            { "Net Gain/Loss from Crimes", RR::utility::currencyToStr(-gain) }
        });
    }
}
