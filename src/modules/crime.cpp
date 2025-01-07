#include "crime.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigroles.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/extensions/cache.h"
#include "dppinteract/interactiveservice.h"
#include "investments.h"
#include "systems/itemsystem.h"
#include "utils/ld.h"
#include "utils/random.h"
#include "utils/regex.h"
#include "utils/strings.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>

Crime::Crime() : dppcmd::module<Crime>("Crime", "Hell yeah! Crime! Reject the ways of being a law-abiding citizen for some cold hard cash and maybe even a tool. Or, maybe not. Depends how good you are at being a criminal.")
{
    register_command(&Crime::bully, std::in_place, "bully", "Change the nickname of any victim you wish.", "$bully [user] [nickname]");
    register_command(&Crime::deal, std::in_place, "deal", "Deal some drugs.");
    register_command(&Crime::hack, std::in_place, "hack", "Hack into someone's crypto wallet.", "$hack [user] [crypto] [amount]");
    register_command(&Crime::loot, std::in_place, "loot", "Loot some locations.");
    register_command(&Crime::rape, std::in_place, { "rape", "strugglesnuggle" }, "Get yourself some ass!", "$rape [user]");
    register_command(&Crime::rob, std::in_place, "rob", "Yoink money from a user.", "$rob [user] [amount]");
    register_command(&Crime::scavenge, std::in_place, "scavenge", "Scavenge around the streets for some cash money.");
    register_command(&Crime::slavery, std::in_place, "slavery", "Get some slave labor goin'.");
    register_command(&Crime::whore, std::in_place, "whore", "Sell your body for quick cash.");
}

dpp::task<dppcmd::command_result> Crime::bully(dpp::guild_member member, const dppcmd::remainder<std::string>& nickname)
{
    if (nickname->size() > 32)
        co_return dppcmd::command_result::from_error(Responses::NicknameTooLong);

    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (target.perks.contains("Pacifist"))
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasPacifist, "bully", member.get_mention()));

    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    if (roles.memberIsStaff(member))
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsStaff, "bully", member.get_mention()));

    std::string nicknameValue = *nickname;
    member.set_nickname(nicknameValue);
    co_await cluster->co_guild_edit_member(member);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    author.modCooldown(author.bullyCooldown = Constants::BullyCooldown, member);
    MongoManager::updateUser(author);

    co_return dppcmd::command_result::from_success(std::format(Responses::Bullied, member.get_mention(), RR::utility::sanitize(nicknameValue)));
}

dpp::task<dppcmd::command_result> Crime::deal()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::DealSuccesses, Responses::DealFails, user,
                                    user.dealCooldown = Constants::DealCooldown, true);
}

dpp::task<dppcmd::command_result> Crime::hack(const dpp::guild_member& member, const std::string& crypto, long double amount)
{
    amount = RR::utility::round(amount, 4);
    if (amount < Constants::InvestmentMinAmount)
        co_return dppcmd::command_result::from_error(std::format(Responses::HackTooSmall, Constants::InvestmentMinAmount));

    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::BadIdea);

    std::string abbrev = Investments::resolveAbbreviation(crypto);
    std::string abbrevUpper = RR::utility::toUpper(abbrev);
    if (abbrev.empty())
        co_return dppcmd::command_result::from_error(Responses::InvalidCurrency);

    auto authorMember = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));
    if (target.perks.contains("Pacifist"))
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasPacifist, "hack", member.get_mention()));

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    long double* authorCrypto = author.getCrypto(abbrev);
    long double* targetCrypto = target.getCrypto(abbrev);

    long double hackMax = RR::utility::round(*targetCrypto / 100.0L * Constants::HackMaxPercent, 4);
    if (*authorCrypto < amount)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, abbrevUpper));
    if (amount > hackMax)
        co_return dppcmd::command_result::from_error(std::format(Responses::HackTooLarge, Constants::HackMaxPercent, member.get_mention(), abbrevUpper, RR::utility::roundAsStr(hackMax, 4)));

    std::optional<long double> cryptoValue = co_await Investments::queryCryptoValue(abbrev, cluster);
    if (!cryptoValue)
        co_return dppcmd::command_result::from_error(Responses::GetCryptoValueFailed);

    double odds = Constants::HackOdds;
    if (author.usedConsumables["Black Hat"] > 0)
        odds += 10 * author.usedConsumables["Black Hat"];
    if (author.perks.contains("Speed Demon"))
        odds *= 0.95;

    std::string response;
    std::string mention = member.get_mention();

    if (RR::utility::random(100) < odds)
    {
        *authorCrypto += amount;
        *targetCrypto -= amount;
        statUpdate(author, true, cryptoValue.value() * amount);
        statUpdate(target, false, cryptoValue.value() * amount);

        std::string amountRounded = RR::utility::roundAsStr(amount, 4);
        response = std::vformat(RR::utility::randomElement(Responses::HackSuccesses),
                                std::make_format_args(mention, amountRounded, abbrevUpper));
    }
    else
    {
        *authorCrypto -= amount / 4.0L;
        statUpdate(author, false, cryptoValue.value() * (amount / 4.0L));

        std::string amountRounded = RR::utility::roundAsStr(amount / 4.0L, 4);
        response = std::vformat(RR::utility::randomElement(Responses::HackFails),
                                std::make_format_args(mention, amountRounded, abbrevUpper));
    }

    author.modCooldown(author.hackCooldown = Constants::HackCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);
    co_return dppcmd::command_result::from_success(response);
}

dpp::task<dppcmd::command_result> Crime::loot()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::LootSuccesses, Responses::LootFails, user,
                                    user.lootCooldown = Constants::LootCooldown, true);
}

dpp::task<dppcmd::command_result> Crime::rape(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));
    if (target.perks.contains("Pacifist"))
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasPacifist, "rape", member.get_mention()));
    if (target.cash < 0.01L)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsBroke, member.get_mention()));

    auto authorMember = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    double odds = Constants::RapeOdds;
    if (author.usedConsumables["Viagra"] > 0)
        odds += 10 * author.usedConsumables["Viagra"];
    if (author.perks.contains("Speed Demon"))
        odds *= 0.95;

    long double rapePercent = RR::utility::random(Constants::RapeMinPercent, Constants::RapeMaxPercent + 1);
    if (RR::utility::random(100) < odds)
    {
        long double repairs = target.cash / 100.0L * rapePercent;
        statUpdate(target, false, repairs);
        context->reply(std::format(Responses::RapeSuccess, member.get_mention(), RR::utility::cash2str(repairs)));
        co_await target.setCashWithoutAdjustment(member, target.cash - repairs, cluster);
    }
    else
    {
        long double repairs = author.cash / 100.0L * rapePercent;
        statUpdate(author, false, repairs);
        context->reply(std::format(Responses::RapeFailed, member.get_mention(), RR::utility::cash2str(repairs)));
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash - repairs, cluster);
    }

    author.modCooldown(author.rapeCooldown = Constants::RapeCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);

    co_return dppcmd::command_result::from_success();
}

dpp::task<dppcmd::command_result> Crime::rob(const dpp::guild_member& member, long double amount)
{
    if (amount < Constants::RobMinCash)
        co_return dppcmd::command_result::from_error(std::format(Responses::RobTooSmall, RR::utility::cash2str(Constants::RobMinCash)));
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));
    if (target.perks.contains("Pacifist"))
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasPacifist, "rape", member.get_mention()));

    long double robMax = target.cash / 100.0L * Constants::RobMaxPercent;
    if (amount > robMax)
    {
        co_return dppcmd::command_result::from_error(std::format(Responses::RobTooLarge, Constants::RobMaxPercent,
            member.get_mention(), RR::utility::cash2str(robMax)));
    }

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.cash < amount)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    auto authorMember = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    double odds = Constants::RobOdds;
    if (author.usedConsumables["Ski Mask"] > 0)
        odds += 10 * author.usedConsumables["Ski Mask"];
    if (author.perks.contains("Speed Demon"))
        odds *= 0.95;

    std::string response;
    std::string amountStr = RR::utility::cash2str(amount);
    std::string mention = member.get_mention();

    if (RR::utility::random(100) < odds)
    {
        co_await target.setCashWithoutAdjustment(member, target.cash - amount, cluster);
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash + amount, cluster);
        statUpdate(author, true, amount);
        statUpdate(target, false, amount);

        response = std::vformat(RR::utility::randomElement(Responses::RobSuccesses), std::make_format_args(mention, amountStr));
        response += "\nBalance: " + RR::utility::cash2str(author.cash);
    }
    else
    {
        co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash - amount, cluster);
        statUpdate(author, false, amount);

        response = std::vformat(RR::utility::randomElement(Responses::RobFails), std::make_format_args(mention, amountStr));
        response += "\nBalance: " + RR::utility::cash2str(author.cash);
    }

    author.modCooldown(author.robCooldown = Constants::RobCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);

    co_return dppcmd::command_result::from_success(response);
}

std::string scrambledMatch(const RR::utility::svmatch& m)
{
    std::string str = m.str();

    for (size_t i = 0; i < str.size(); ++i)
    {
        size_t j = RR::utility::random<size_t>(0, str.size() - 1);
        std::swap(str[i], str[j]);
    }

    return str;
}

dpp::task<dppcmd::command_result> Crime::scavenge()
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    auto [word, espanol] = RR::utility::randomElement(Constants::ScavengeWordSet);
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    dpp::embed embed = dpp::embed().set_color(dpp::colors::red);

    if (RR::utility::random(2) == 0)
    {
        static std::regex alnumRegex("\\w+");
        std::string scrambled = RR::utility::regex_replace(word.begin(), word.end(), alnumRegex, scrambledMatch);
        while (dppcmd::utility::iequals(scrambled, word) && scrambled != "egg")
            scrambled = RR::utility::regex_replace(word.begin(), word.end(), alnumRegex, scrambledMatch);

        embed.set_title("Scramble!");
        embed.set_description(std::format(Responses::ScavengeScrambleDescription, scrambled, Constants::ScavengeTimeout));

        if (scrambled == "egg")
        {
            context->reply(Responses::HardBoiledEgg);
            user.unlockAchievement("Hard Boiled Egg", context);
        }
    }
    else
    {
        embed.set_title("Translate!");
        embed.set_description(std::format(Responses::ScavengeTranslateDescription, espanol, Constants::ScavengeTimeout));
    }

    dpp::message inMsg = dpp::message(context->msg.channel_id, embed).set_reference(context->msg.id);
    dpp::confirmation_callback_t event = co_await cluster->co_message_create(inMsg);
    dpp::message outMsg = event.get<dpp::message>();

    auto interactive = extra_data<dppinteract::interactive_service*>();
    dppinteract::interactive_result<dpp::message> result = co_await interactive->next_message([this](const dpp::message& m) {
        return m.channel_id == context->msg.channel_id && m.author.id == context->msg.author.id;
    }, std::chrono::seconds(Constants::ScavengeTimeout));

    std::string content = result.value ? result.value->content : std::string();
    co_await handleScavenge(outMsg, result, user, member.value(), dppcmd::utility::iequals(content, word),
                            std::format(Responses::ScavengeSuccess, word),
                            std::format(Responses::ScavengeTimeout, Constants::ScavengeTimeout, word),
                            std::format(Responses::ScavengeFail, word));

    if (RR::utility::random(50) == 1)
        ItemSystem::giveCollectible("Ape NFT", context, user);

    user.modCooldown(user.scavengeCooldown = Constants::ScavengeCooldown, member.value());
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}

dpp::task<dppcmd::command_result> Crime::slavery()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::SlaverySuccesses, Responses::SlaveryFails, user,
                                    user.slaveryCooldown = Constants::SlaveryCooldown);
}

dpp::task<dppcmd::command_result> Crime::whore()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    co_return co_await genericCrime(Responses::WhoreSuccesses, Responses::WhoreFails, user,
                                    user.whoreCooldown = Constants::WhoreCooldown);
}

dpp::task<dppcmd::command_result> Crime::genericCrime(const std::span<const std::string_view>& successOutcomes,
                                                      const std::span<const std::string_view>& failOutcomes,
                                                      DbUser& user, int64_t& cooldown, bool hasMehOutcome)
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    double winOdds = user.perks.contains("Speed Demon")
        ? Constants::GenericCrimeWinOdds * 0.95 : Constants::GenericCrimeWinOdds;

    std::string outcome;
    long double totalCash;
    if (RR::utility::random(100) < winOdds)
    {
        size_t outcomeNum = RR::utility::random(successOutcomes.size());
        long double moneyEarned = RR::utility::random(Constants::GenericCrimeWinMin, Constants::GenericCrimeWinMax);
        if (hasMehOutcome && outcomeNum == successOutcomes.size() - 1)
            moneyEarned /= 5;

        std::string curr = RR::utility::cash2str(moneyEarned);
        outcome = std::vformat(successOutcomes[outcomeNum], std::make_format_args(curr));
        totalCash = user.cash + moneyEarned;
        statUpdate(user, true, moneyEarned);
    }
    else
    {
        long double lostCash = RR::utility::random(Constants::GenericCrimeLossMin, Constants::GenericCrimeLossMax);
        if (lostCash > user.cash)
            lostCash = user.cash;

        std::string curr = RR::utility::cash2str(lostCash);
        outcome = std::vformat(RR::utility::randomElement(failOutcomes), std::make_format_args(curr));
        totalCash = user.cash - lostCash;
        statUpdate(user, false, lostCash);
    }

    co_await user.setCash(member.value(), totalCash, cluster, context,
                          std::format("{}\nBalance: {}", outcome, RR::utility::cash2str(totalCash)));

    if (RR::utility::random(100) < Constants::GenericCrimeToolOdds)
    {
        auto availableTools = Constants::Tools
            | std::views::filter([&user](const Tool& t) { return !user.tools.contains(t.name()); })
            | std::views::transform([](const Tool& t) { return t.name(); });

        if (!std::ranges::empty(availableTools))
        {
            std::string_view tool = RR::utility::randomElement(availableTools);
            user.tools.emplace(tool);
            context->reply(std::format(Responses::GotTool, tool));
        }
    }

    user.modCooldown(cooldown, member.value());
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}

dpp::task<void> Crime::handleScavenge(dpp::message& msg, const dppinteract::interactive_result<dpp::message>& result,
                                      DbUser& user, const dpp::guild_member& member,
                                      bool successCondition, std::string_view successResponse,
                                      std::string_view timeoutResponse, std::string_view failureResponse)
{
    if (result.timed_out())
    {
        msg.embeds.front().set_description(std::string(timeoutResponse));
    }
    else if (successCondition)
    {
        long double rewardCash = RR::utility::random(Constants::ScavengeMinCash, Constants::ScavengeMaxCash);
        long double prestigeCash = rewardCash * 0.2L * user.prestige;
        long double totalCash = user.cash + rewardCash + prestigeCash;

        std::string response(successResponse);
        response += std::format(" Here's {}.\nBalance: {}",
                                RR::utility::cash2str(rewardCash), RR::utility::cash2str(totalCash));
        if (prestigeCash > 0)
            response += std::format("\n*(+{} from Prestige)*", RR::utility::cash2str(prestigeCash));

        co_await user.setCashWithoutAdjustment(member, totalCash, cluster);
        msg.embeds.front().set_description(response);
    }
    else
    {
        msg.embeds.front().set_description(std::string(failureResponse));
    }

    co_await cluster->co_message_edit(msg);
}

void Crime::statUpdate(DbUser& user, bool success, long double gain)
{
    std::string gainStr = RR::utility::cash2str(gain);
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
            { "Net Gain/Loss from Crimes", RR::utility::cash2str(-gain) }
        });
    }
}
