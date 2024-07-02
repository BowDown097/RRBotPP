#include "gambling.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "utils/ld.h"
#include "utils/random.h"
#include "utils/timestamp.h"
#include <dpp/cache.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>

Gambling::Gambling() : dpp::module<Gambling>("Gambling", "Do you want to test your luck? Do you want to probably go broke? Here you go! By the way, you don't need to be 21 or older in this joint ;)")
{
    register_command(&Gambling::roll55, "55x2", "Roll 55 or higher on a 100 sided die, get 2x what you put in.", "$55x2 [bet]");
    register_command(&Gambling::roll6969, "6969", "Roll 69.69 on a 100 sided die, get 6969x what you put in.", "$6969 bet");
    register_command(&Gambling::roll75, "75+", "Roll 75 or higher on a 100 sided die, get 3.6x what you put in.", "$75+ [bet]");
    register_command(&Gambling::roll99, "99+", "Roll 99 or higher on a 100 sided die, get 90x what you put in.",  "$99+ [bet]");
    register_command(&Gambling::dice, "dice", "Play a simple game of Chuck-a-luck, AKA Birdcage. If you don't know how it works: The player bets on a number. Three dice are rolled. The number appearing once gives a 1:1 payout, twice a 2:1, and thrice a 10:1.", "$dice [bet] [number]");
    register_command(&Gambling::doubleGamble, "double", "Double your cash...?");
    register_command(&Gambling::pot, "pot", "View the pot or add money into it.", "$pot <bet>");
}

dpp::task<dpp::command_result> Gambling::dice(const cash_in& betIn, int number)
{
    long double bet = betIn.top_result();
    if (bet < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::curr2str(Constants::TransactionMin)));
    if (number < 1 || number > 6)
        co_return dpp::command_result::from_error(Responses::InvalidDice);

    std::optional<dpp::guild_member> member = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

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
        description += std::format(Responses::Dice2Matches, RR::utility::curr2str(bet * 2));
        break;
    case 3:
        payout = bet * 9;
        description += std::format(Responses::Dice3Matches, RR::utility::curr2str(bet * 10));
        break;
    default:
        description += std::format(Responses::DiceNoMatches, RR::utility::curr2str(bet));
        break;
    }

    long double totalCash = user.cash + payout;
    description += "\nBalance: " + RR::utility::curr2str(totalCash);

    if (matches == 3)
        user.unlockAchievement("OH BABY A TRIPLE", context);

    if (user.gamblingMultiplier > 1)
    {
        long double multiplierCash = payout * user.gamblingMultiplier - payout;
        description += std::format("\n*(+{} from gambling multiplier)*", RR::utility::curr2str(multiplierCash));
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
    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Gambling::doubleGamble()
{
    std::optional<dpp::guild_member> member = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

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
    co_return dpp::command_result::from_success(Responses::Doubled);
}

dpp::task<dpp::command_result> Gambling::pot(const std::optional<cash_in>& betIn)
{
    if (!betIn)
    {
        DbPot pot = MongoManager::fetchPot(context->msg.guild_id);
        if (pot.endTime < RR::utility::unixTimestamp())
            co_return dpp::command_result::from_error("The pot is currently empty.");

        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::red)
            .set_title("Pot")
            .add_field("Total Value", RR::utility::curr2str(pot.value))
            .add_field("Draws At", dpp::utility::timestamp(pot.endTime));

        std::string memberInfo;
        for (const auto& [userId, bet] : pot.members)
        {
            if (dpp::user* user = dpp::find_user(userId))
                memberInfo += std::format("**{}**: {} ({:.2f}%)\n", user->get_mention(), RR::utility::curr2str(bet), pot.getMemberOdds(userId));
            else
                memberInfo += std::format("**???**: {} ({:.2f}%)\n", RR::utility::curr2str(bet), pot.getMemberOdds(userId));
        }

        if (!memberInfo.empty())
            memberInfo.pop_back();

        embed.add_field("Members", memberInfo);
        context->reply(dpp::message(context->msg.channel_id, embed));
        co_return dpp::command_result::from_success();
    }

    long double bet = betIn->top_result();
    if (bet < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::curr2str(Constants::TransactionMin)));

    std::optional<dpp::guild_member> member = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

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

    co_return dpp::command_result::from_success(std::format(Responses::AddedIntoPot, RR::utility::curr2str(bet)));
}

dpp::task<dpp::command_result> Gambling::roll55(const cash_in& betIn) { co_return co_await genericGamble(betIn.top_result(), 55, 1); }
dpp::task<dpp::command_result> Gambling::roll6969(const cash_in& betIn) { co_return co_await genericGamble(betIn.top_result(), 69.69L, 6968, true); }
dpp::task<dpp::command_result> Gambling::roll75(const cash_in& betIn) { co_return co_await genericGamble(betIn.top_result(), 75, 2.6L); }
dpp::task<dpp::command_result> Gambling::roll99(const cash_in& betIn) { co_return co_await genericGamble(betIn.top_result(), 99, 89); }

dpp::task<dpp::command_result> Gambling::genericGamble(long double bet, long double odds, long double mult, bool exactRoll)
{
    if (bet < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "bet", RR::utility::curr2str(Constants::TransactionMin)));

    std::optional<dpp::guild_member> member = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < bet)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    long double roll = RR::utility::round(RR::utility::random(1.0L, 100.0L), 2);
    if (!exactRoll && user.perks.contains("Speed Demon"))
        odds *= 1.05L;

    if (exactRoll ? roll == odds : roll >= odds)
    {
        long double payout = bet * mult;
        long double totalCash = user.cash + payout;
        statUpdate(user, true, payout);
        std::string message = std::format(Responses::GenericGambleSuccess, roll,
                                          RR::utility::curr2str(payout), RR::utility::curr2str(totalCash));

        if (odds >= 99)
            user.unlockAchievement("Pretty Damn Lucky", context);
        else if (odds == 69.69L)
            user.unlockAchievement("Luckiest Dude Alive", context);

        if (user.gamblingMultiplier > 1)
        {
            long double multiplierCash = payout * user.gamblingMultiplier - payout;
            message += std::format("\n*(+{} from gambling multiplier)*", RR::utility::curr2str(multiplierCash));
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
                                          RR::utility::curr2str(bet), RR::utility::curr2str(totalCash));
        co_await user.setCashWithoutAdjustment(member.value(), totalCash, cluster, context, message);
    }

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success();
}

void Gambling::statUpdate(DbUser& user, bool success, long double gain)
{
    std::string gainStr = RR::utility::curr2str(gain);
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
            { "Net Gain/Loss from Gambling", RR::utility::curr2str(-gain) }
        });
    }
}
