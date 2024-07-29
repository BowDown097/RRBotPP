#include "investments.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "utils/ld.h"
#include "utils/strings.h"
#include <chrono>
#include <dpp/cluster.h>
#include <dpp/colors.h>

Investments::Investments() : dpp::module<Investments>("Investments", "Invest in our selection of coins, Bit or Shit. The prices here are updated in REAL TIME with REAL WORLD values. Experience the fast, entrepreneural life without going broke, having your house repossessed, and having your girlfriend leave you. Wait, you probably don't have either of those.")
{
    register_command(&Investments::invest, "invest", "Invest in a cryptocurrency. Currently accepted currencies are BTC, ETH, LTC, and XRP.", "$invest [crypto] [cash amount]");
    register_command(&Investments::investments, "investments", "Check your investments, or someone else's, and their value.", "$investments <user>");
    register_command(&Investments::prices, std::initializer_list<std::string> { "prices", "values" }, "Check the values of the currently avaiable cryptocurrencies.");
    register_command(&Investments::withdraw, "withdraw", "Withdraw a specified cryptocurrency to cash, with a 2% withdrawal fee. See $invest's info for currently accepted currencies.", "$withdraw [crypto] [amount]");
}

dpp::task<dpp::command_result> Investments::invest(const std::string& crypto, const cash_in& cashAmountIn)
{
    long double cashAmount = cashAmountIn.top_result();
    if (cashAmount < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "invest", RR::utility::cash2str(Constants::TransactionMin)));

    std::string abbrev = resolveAbbreviation(crypto);
    std::string abbrevUpper = RR::utility::toUpper(abbrev);
    if (abbrev.empty())
        co_return dpp::command_result::from_error(Responses::InvalidCrypto);

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < cashAmount)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    std::optional<long double> cryptoValue = co_await queryCryptoValue(abbrev, cluster);
    if (!cryptoValue)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    long double cryptoAmount = RR::utility::round(cashAmount / cryptoValue.value(), 4);
    if (cryptoAmount < Constants::InvestmentMinAmount)
    {
        co_return dpp::command_result::from_error(std::format(Responses::InvestmentTooLow,
            Constants::InvestmentMinAmount, abbrevUpper,
            RR::utility::cash2str(cryptoValue.value() * Constants::InvestmentMinAmount)));
    }

    co_await user.setCashWithoutAdjustment(gm.value(), user.cash - cashAmount, cluster);
    *user.getCrypto(abbrev) += cryptoAmount;
    user.mergeStats({
        { "Money Put Into " + abbrevUpper, RR::utility::cash2str(cashAmount) },
        { abbrevUpper + " Purchased", dpp::utility::lexical_cast<std::string>(cryptoAmount) }
    });

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::InvestmentSuccess,
        RR::utility::roundAsStr(cryptoAmount, 4), abbrevUpper, RR::utility::cash2str(cashAmount)));
}

dpp::task<dpp::command_result> Investments::investments(const std::optional<dpp::guild_member_in>& memberOpt)
{
    std::optional<dpp::guild_member> member = memberOpt
        ? memberOpt->top_result() : dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    dpp::user* user = member->get_user();
    if (!user)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->is_bot())
        co_return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);

    std::string investsDisplay;
    if (dbUser.btc >= 0.01L)
    {
        std::optional<long double> btcValue = co_await queryCryptoValue("BTC", cluster);
        if (btcValue)
        {
            investsDisplay += std::format("**BTC**: {} ({})\n",
                RR::utility::roundAsStr(dbUser.btc, 4), RR::utility::cash2str(btcValue.value() * dbUser.btc));
        }
    }
    if (dbUser.eth >= 0.01L)
    {
        std::optional<long double> ethValue = co_await queryCryptoValue("ETH", cluster);
        if (ethValue)
        {
            investsDisplay += std::format("**ETH**: {} ({})\n",
                RR::utility::roundAsStr(dbUser.eth, 4), RR::utility::cash2str(ethValue.value() * dbUser.eth));
        }
    }
    if (dbUser.ltc >= 0.01L)
    {
        std::optional<long double> ltcValue = co_await queryCryptoValue("LTC", cluster);
        if (ltcValue)
        {
            investsDisplay += std::format("**LTC**: {} ({})\n",
                RR::utility::roundAsStr(dbUser.ltc, 4), RR::utility::cash2str(ltcValue.value() * dbUser.ltc));
        }
    }
    if (dbUser.xrp >= 0.01L)
    {
        std::optional<long double> xrpValue = co_await queryCryptoValue("XRP", cluster);
        if (xrpValue)
        {
            investsDisplay += std::format("**XRP**: {} ({})\n",
                RR::utility::roundAsStr(dbUser.xrp, 4), RR::utility::cash2str(xrpValue.value() * dbUser.xrp));
        }
    }

    if (!investsDisplay.empty())
        investsDisplay.pop_back(); // remove trailing newline

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Investments")
        .set_description(!investsDisplay.empty() ? investsDisplay : "None");

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Investments::prices()
{
    std::optional<long double> btc = co_await queryCryptoValue("BTC", cluster);
    if (!btc)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    std::optional<long double> eth = co_await queryCryptoValue("ETH", cluster);
    if (!eth)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    std::optional<long double> ltc = co_await queryCryptoValue("LTC", cluster);
    if (!ltc)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    std::optional<long double> xrp = co_await queryCryptoValue("XRP", cluster);
    if (!xrp)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Cryptocurrency Values")
        .add_field("Bitcoin (BTC)", RR::utility::cash2str(btc.value()))
        .add_field("Ethereum (ETH)", RR::utility::cash2str(eth.value()))
        .add_field("Litecoin (LTC)", RR::utility::cash2str(ltc.value()))
        .add_field("XRP", RR::utility::cash2str(xrp.value()));

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Investments::withdraw(const std::string& crypto, long double cryptoAmount)
{
    cryptoAmount = RR::utility::round(cryptoAmount, 4);
    if (cryptoAmount < Constants::InvestmentMinAmount)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "withdraw", Constants::InvestmentMinAmount));

    std::string abbrev = resolveAbbreviation(crypto);
    std::string abbrevUpper = RR::utility::toUpper(abbrev);
    if (abbrev.empty())
        co_return dpp::command_result::from_error(Responses::InvalidCrypto);

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    long double* cryptoBal = user.getCrypto(abbrev);
    if (*cryptoBal < cryptoAmount)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, abbrevUpper));

    std::optional<long double> cryptoValue = co_await queryCryptoValue(abbrev, cluster);
    if (!cryptoValue)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    long double totalCashValue = cryptoValue.value() * cryptoAmount;
    long double finalCashValue = totalCashValue / 100.0L * (100.0L - Constants::InvestmentFeePercent);
    std::string totalCashValueStr = RR::utility::cash2str(totalCashValue);
    std::string finalCashValueStr = RR::utility::cash2str(finalCashValue);

    co_await user.setCashWithoutAdjustment(gm.value(), user.cash + finalCashValue, cluster);
    *cryptoBal -= cryptoAmount;
    user.mergeStat("Money Gained from " + abbrevUpper, finalCashValueStr);

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::WithdrawSuccess,
        RR::utility::roundAsStr(cryptoAmount, 4), abbrevUpper, totalCashValueStr,
        Constants::InvestmentFeePercent, finalCashValueStr));
}

dpp::task<std::optional<long double>> Investments::queryCryptoValue(std::string_view crypto, dpp::cluster* cluster)
{
    std::string today = std::format("{:%Y-%m-%d}T00:00", std::chrono::system_clock::now());
    std::string current = std::format("{:%Y-%m-%dT%H:%M}", std::chrono::system_clock::now());
    dpp::http_request_completion_t result = co_await cluster->co_request(
        std::format("https://production.api.coindesk.com/v2/price/values/{}?start_date={}&end_date={}",
                    RR::utility::toUpper(crypto), today, current), dpp::m_get);

    nlohmann::json json = nlohmann::json::parse(result.body, nullptr, false);
    if (!json["data"]["entries"].is_array())
        co_return std::nullopt;

    nlohmann::json& lastEntry = json["data"]["entries"].back();
    if (!lastEntry[1].is_number())
        co_return std::nullopt;

    co_return RR::utility::round(lastEntry[1].template get<long double>(), 2);
}

std::string Investments::resolveAbbreviation(std::string_view crypto)
{
    std::string cryptoLower = RR::utility::toLower(crypto);
    if (cryptoLower == "bitcoin" || cryptoLower == "btc")
        return "btc";
    else if (cryptoLower == "ethereum" || cryptoLower == "eth")
        return "eth";
    else if (cryptoLower == "litecoin" || cryptoLower == "ltc")
        return "ltc";
    else if (cryptoLower == "xrp")
        return "xrp";
    return std::string();
}
