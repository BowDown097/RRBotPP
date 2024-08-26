#include "gambling.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-interactive/interactiveservice.h"
#include "dppcmd/extensions/cache.h"
#include "utils/ld.h"
#include "utils/random.h"
#include "utils/timestamp.h"
#include <dpp/cache.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>

Gambling::Gambling() : dppcmd::module<Gambling>("Gambling", "Do you want to test your luck? Do you want to probably go broke? Here you go! By the way, you don't need to be 21 or older in this joint ;)")
{
    register_command(&Gambling::bet, "bet", "Pick a number between 1 and 100 and place a bet on it against another user. The user and I will also pick a number between 1 and 100. Whoever is closest to the number I pick wins!", "$bet [user] [bet] [number]");
    register_command(&Gambling::roll55, "55x2", "Roll 55 or higher, get 2x what you put in.", "$55x2 [bet]");
    register_command(&Gambling::roll6969, "6969", "Roll 69.69, get 6969x what you put in.", "$6969 [bet]");
    register_command(&Gambling::roll75, "75+", "Roll 75 or higher, get 3.6x what you put in.", "$75+ [bet]");
    register_command(&Gambling::roll99, "99+", "Roll 99 or higher, get 90x what you put in.",  "$99+ [bet]");
    register_command(&Gambling::dice, "dice", "Play a simple game of Chuck-a-luck, AKA Birdcage. If you don't know how it works: The player bets on a number. Three dice are rolled. The number appearing once gives a 1:1 payout, twice a 2:1, and thrice a 10:1.", "$dice [bet] [number]");
    register_command(&Gambling::doubleGamble, "double", "Double your cash...?");
    register_command(&Gambling::pot, "pot", "View the pot or add money into it.", "$pot <bet>");
}

// why does no usingSlots-like restriction apply to this command?
// because you can involve another user in this, that would open up the opportunity
// for a committed retard to essentially lock someone out of the bot.
// $slots is also probably the heaviest command in the entire bot,
// which is the other reason why i introduced the restriction.
dpp::task<dppcmd::command_result> Gambling::bet(const dpp::guild_member& member, long double bet, int number)
{
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (bet < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::cash2str(Constants::TransactionMin)));
    if (number < 1 || number > 100)
        co_return dppcmd::command_result::from_error(std::format(Responses::InputNotInRange, 1, 100));
    if (dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    auto authorMember = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser authorDbUser = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (authorDbUser.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    DbUser targetDbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (targetDbUser.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasNotEnoughOfThing, member.get_mention(), "cash"));
    if (targetDbUser.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));

    std::string betStr = RR::utility::cash2str(bet);
    dpp::embed betEmbed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Bet")
        .set_description(std::format(Responses::BetDescription,
            context->msg.author.get_mention(), betStr, member.get_mention(), number, member.get_mention()));
    context->reply(dpp::message(context->msg.channel_id, betEmbed));

    int targetNumber;
    dpp::interactive_service* interactive = extra_data<dpp::interactive_service*>();
    auto betResult = co_await interactive->next_message([this, &targetNumber, userId = member.user_id](const dpp::message& m) {
        targetNumber = dppcmd::utility::lexical_cast<int>(m.content, false);
        return m.channel_id == context->msg.channel_id && m.author.id == userId && targetNumber >= 1 && targetNumber <= 100;
    });

    if (!betResult.success() || !betResult.value)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserDidntRespond, member.get_mention()));
    if (targetNumber == number)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserPickedSameNumber, member.get_mention()));

    // now, we get the dbusers AGAIN in case something has changed
    authorDbUser = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (authorDbUser.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));
    if (authorDbUser.usingSlots)
        co_return dppcmd::command_result::from_error(Responses::YouAreGambling);

    targetDbUser = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (targetDbUser.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserHasNotEnoughOfThing, member.get_mention(), "cash"));
    if (targetDbUser.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));

    int botNumber = RR::utility::random(1, 101);
    int authorDistance = std::abs(botNumber - number);
    int targetDistance = std::abs(botNumber - targetNumber);

    std::string response;
    if (authorDistance < targetDistance)
    {
        co_await authorDbUser.setCashWithoutAdjustment(authorMember.value(), authorDbUser.cash + bet, cluster);
        co_await targetDbUser.setCashWithoutAdjustment(member, targetDbUser.cash - bet, cluster);
        response = std::format(Responses::BetResult, context->msg.author.get_mention(), botNumber, betStr);
        MongoManager::updateUser(authorDbUser);
    }
    else
    {
        co_await targetDbUser.setCashWithoutAdjustment(member, targetDbUser.cash + bet, cluster);
        response = std::format(Responses::BetResult, member.get_mention(), botNumber, betStr);
    }

    MongoManager::updateUser(targetDbUser);
    co_return dppcmd::command_result::from_success(response);
}

dpp::task<dppcmd::command_result> Gambling::dice(long double bet, int number)
{
    if (bet < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::cash2str(Constants::TransactionMin)));
    if (number < 1 || number > 6)
        co_return dppcmd::command_result::from_error(std::format(Responses::InputNotInRange, 1, 6));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    int rolls[3] = { RR::utility::random(1, 7), RR::utility::random(1, 7), RR::utility::random(1, 7) };
    int matches = std::ranges::count(rolls, number);
    std::string description = std::format("{} {} {}\n\n", rolls[0], rolls[1], rolls[2]);

    long double payout = -bet;
    switch (matches)
    {
    case 1:
        payout = 0;
        description += Responses::Dice1Match;
        break;
    case 2:
        payout = bet;
        description += std::format(Responses::Dice2Matches, RR::utility::cash2str(bet * 2));
        break;
    case 3:
        payout = bet * 9;
        description += std::format(Responses::Dice3Matches, RR::utility::cash2str(bet * 10));
        break;
    default:
        description += std::format(Responses::DiceNoMatches, RR::utility::cash2str(bet));
        break;
    }

    long double totalCash = user.cash + payout;
    description += "\nBalance: " + RR::utility::cash2str(totalCash);

    if (matches == 3)
        user.unlockAchievement("OH BABY A TRIPLE", context);

    if (user.gamblingMultiplier > 1)
    {
        long double multiplierCash = payout * user.gamblingMultiplier - payout;
        description += std::format("\n*(+{} from gambling multiplier)*", RR::utility::cash2str(multiplierCash));
        totalCash += multiplierCash;
    }

    statUpdate(user, matches > 0, payout);
    co_await user.setCashWithoutAdjustment(member.value(), totalCash, cluster);
    MongoManager::updateUser(user);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::DiceTitle)
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dppcmd::command_result::from_success();
}

dpp::task<dppcmd::command_result> Gambling::doubleGamble()
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (RR::utility::random(100) < Constants::DoubleOdds)
    {
        statUpdate(user, true, user.cash);
        co_await user.setCashWithoutAdjustment(member.value(), user.cash * 2, cluster);
    }
    else
    {
        statUpdate(user, false, user.cash);
        co_await user.setCashWithoutAdjustment(member.value(), 0, cluster);
    }

    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success(Responses::Doubled);
}

dpp::task<dppcmd::command_result> Gambling::pot(const std::optional<long double>& betOpt)
{
    if (!betOpt)
    {
        DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
        if (pot.endTime < RR::utility::unixTimestamp())
            co_return dppcmd::command_result::from_error("The pot is currently empty.");

        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::red)
            .set_title("Pot")
            .add_field("Total Value", RR::utility::cash2str(pot.value))
            .add_field("Draws At", dpp::utility::timestamp(pot.endTime));

        std::string memberInfo;
        for (const auto& [userId, bet] : pot.members)
        {
            if (dpp::user* user = dpp::find_user(userId))
                memberInfo += std::format("**{}**: {} ({:.2f}%)\n", user->get_mention(), RR::utility::cash2str(bet), pot.getMemberOdds(userId));
            else
                memberInfo += std::format("**???**: {} ({:.2f}%)\n", RR::utility::cash2str(bet), pot.getMemberOdds(userId));
        }

        if (!memberInfo.empty())
            memberInfo.pop_back();

        embed.add_field("Members", memberInfo);
        context->reply(dpp::message(context->msg.channel_id, embed));
        co_return dppcmd::command_result::from_success();
    }

    long double bet = *betOpt;
    if (bet < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::cash2str(Constants::TransactionMin)));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
    if (pot.endTime < RR::utility::unixTimestamp())
    {
        pot.endTime = RR::utility::unixTimestamp(86400);
        pot.members.clear();
        pot.value = 0;
    }

    if (auto it = pot.members.find(context->msg.author.id); it != pot.members.end())
        pot.members[context->msg.author.id] = it->second + bet;
    else
        pot.members[context->msg.author.id] = bet;

    pot.value += bet;
    co_await user.setCashWithoutAdjustment(member.value(), user.cash - bet, cluster);

    MongoManager::updatePot(pot);
    MongoManager::updateUser(user);

    co_return dppcmd::command_result::from_success(std::format(Responses::AddedIntoPot, RR::utility::cash2str(bet)));
}

dpp::task<dppcmd::command_result> Gambling::roll55(long double bet) { co_return co_await genericGamble(bet, 55, 1); }
dpp::task<dppcmd::command_result> Gambling::roll6969(long double bet) { co_return co_await genericGamble(bet, 69.69L, 6968, true); }
dpp::task<dppcmd::command_result> Gambling::roll75(long double bet) { co_return co_await genericGamble(bet, 75, 2.6L); }
dpp::task<dppcmd::command_result> Gambling::roll99(long double bet) { co_return co_await genericGamble(bet, 99, 89); }

dpp::task<dppcmd::command_result> Gambling::genericGamble(long double bet, long double target, long double mult, bool exactRoll)
{
    if (bet < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::cash2str(Constants::TransactionMin)));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    long double roll = RR::utility::round(RR::utility::random(1.0L, 100.0L), 2);
    if (!exactRoll && user.perks.contains("Speed Demon"))
        target *= 1.05L;

    if (exactRoll ? roll == target : roll >= target)
    {
        long double payout = bet * mult;
        long double totalCash = user.cash + payout;
        statUpdate(user, true, payout);
        std::string message = std::format(Responses::GenericGambleSuccess, roll,
                                          RR::utility::cash2str(payout), RR::utility::cash2str(totalCash));

        if (target >= 99)
            user.unlockAchievement("Pretty Damn Lucky", context);
        else if (target == 69.69L)
            user.unlockAchievement("Luckiest Dude Alive", context);

        if (user.gamblingMultiplier > 1)
        {
            long double multiplierCash = payout * user.gamblingMultiplier - payout;
            message += std::format("\n*(+{} from gambling multiplier)*", RR::utility::cash2str(multiplierCash));
            totalCash += multiplierCash;
        }

        co_await user.setCashWithoutAdjustment(member.value(), totalCash, cluster, context, message);
    }
    else
    {
        long double totalCash = user.cash - bet > 0 ? user.cash - bet : 0;
        statUpdate(user, false, bet);
        if (bet >= 1000000)
            user.unlockAchievement("I Just Feel Bad", context);

        std::string message = std::format(Responses::GenericGambleFail, roll,
                                          RR::utility::cash2str(bet), RR::utility::cash2str(totalCash));
        co_await user.setCashWithoutAdjustment(member.value(), totalCash, cluster, context, message);
    }

    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}

void Gambling::statUpdate(DbUser& user, bool success, long double gain)
{
    std::string gainStr = RR::utility::cash2str(gain);
    if (success)
    {
        user.mergeStats(std::unordered_map<std::string, std::string> {
            { "Gambles Won", "1" },
            { "Money Gained from Gambling", gainStr },
            { "Net Gain/Loss from Gambling", gainStr }
        });
    }
    else
    {
        user.mergeStats(std::unordered_map<std::string, std::string> {
            { "Gambles Lost", "1" },
            { "Money Lost to Gambling", gainStr },
            { "Net Gain/Loss from Gambling", RR::utility::cash2str(-gain) }
        });
    }
}
