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
    register_command(&Investments::invest, "invest", "Invest in a cryptocurrency. Currently accepted currencies are BTC, ETH, LTC, and XRP. Here, the amount you put in should be cash, not crypto.", "$invest [crypto] [cash amount]");
    register_command(&Investments::investments, "investments", "Check your investments, or someone else's, and their value.", "$investments <user>");
    register_command(&Investments::prices, std::initializer_list<std::string> { "prices", "values" }, "Check the values of the currently avaiable cryptocurrencies.");
    register_command(&Investments::withdraw, "withdraw", "Withdraw a specified cryptocurrency to cash, with a 2% withdrawal fee. Here, the amount you put in should be crypto, not cash. See $invest's info for currently accepted currencies.", "$withdraw [crypto] [amount]");
}

dpp::task<dpp::command_result> Investments::invest(const std::string& crypto, const cash_in& amountIn)
{
    long double amount = amountIn.top_result();
    if (amount < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "invest", RR::utility::curr2str(Constants::TransactionMin)));

    std::string abbrev = resolveAbbreviation(crypto);
    std::string abbrevUpper = RR::utility::toUpper(abbrev);
    if (abbrev.empty())
        co_return dpp::command_result::from_error(Responses::InvalidCrypto);

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.cash < amount)
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    std::optional<long double> cryptoValue = co_await queryCryptoValue(abbrev, cluster);
    if (!cryptoValue)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    long double cryptoAmount = amount / cryptoValue.value();
    if (cryptoAmount < Constants::InvestmentMinAmount)
    {
        co_return dpp::command_result::from_error(std::format(Responses::InvestmentTooLow,
            Constants::InvestmentMinAmount, abbrevUpper,
            RR::utility::curr2str(cryptoValue.value() * Constants::InvestmentMinAmount)));
    }

    co_await user.setCashWithoutAdjustment(gm.value(), user.cash - amount, cluster);
    *user.getCrypto(abbrev) += RR::utility::round(cryptoAmount, 4);
    user.mergeStats({
        { "Money Put Into " + abbrevUpper, RR::utility::curr2str(amount) },
        { abbrevUpper + " Purchased", dpp::utility::lexical_cast<std::string>(cryptoAmount) }
    });

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::InvestmentSuccess,
        cryptoAmount, abbrevUpper, RR::utility::curr2str(amount)));
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
            investsDisplay += std::format("**BTC**: {:.4f} ({})\n", dbUser.btc, RR::utility::curr2str(btcValue.value() * dbUser.btc));
    }
    if (dbUser.eth >= 0.01L)
    {
        std::optional<long double> ethValue = co_await queryCryptoValue("ETH", cluster);
        if (ethValue)
            investsDisplay += std::format("**ETH**: {:.4f} ({})\n", dbUser.eth, RR::utility::curr2str(ethValue.value() * dbUser.eth));
    }
    if (dbUser.ltc >= 0.01L)
    {
        std::optional<long double> ltcValue = co_await queryCryptoValue("LTC", cluster);
        if (ltcValue)
            investsDisplay += std::format("**LTC**: {:.4f} ({})\n", dbUser.ltc, RR::utility::curr2str(ltcValue.value() * dbUser.ltc));
    }
    if (dbUser.xrp >= 0.01L)
    {
        std::optional<long double> xrpValue = co_await queryCryptoValue("XRP", cluster);
        if (xrpValue)
            investsDisplay += std::format("**XRP**: {:.4f} ({})\n", dbUser.xrp, RR::utility::curr2str(xrpValue.value() * dbUser.xrp));
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
        .add_field("Bitcoin (BTC)", RR::utility::curr2str(btc.value()))
        .add_field("Ethereum (ETH)", RR::utility::curr2str(eth.value()))
        .add_field("Litecoin (LTC)", RR::utility::curr2str(ltc.value()))
        .add_field("XRP", RR::utility::curr2str(xrp.value()));

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Investments::withdraw(const std::string& crypto, long double amount)
{
    if (amount < Constants::InvestmentMinAmount)
        co_return dpp::command_result::from_error(std::format(Responses::CashInputTooLow, "withdraw", Constants::InvestmentMinAmount));

    std::string abbrev = resolveAbbreviation(crypto);
    std::string abbrevUpper = RR::utility::toUpper(abbrev);
    if (abbrev.empty())
        co_return dpp::command_result::from_error(Responses::InvalidCrypto);

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    const long double* cryptoBal = user.getCrypto(abbrev);
    if (*cryptoBal < RR::utility::round(amount, 4))
        co_return dpp::command_result::from_error(std::format(Responses::NotEnoughOfThing, abbrevUpper));

    std::optional<long double> cryptoValue = co_await queryCryptoValue(abbrev, cluster);
    if (!cryptoValue)
        co_return dpp::command_result::from_error(Responses::GetCryptoValueFailed);

    long double totalValue = cryptoValue.value() * amount;
    long double finalValue = totalValue / 100.0L * (100.0L - Constants::InvestmentFeePercent);

    co_await user.setCashWithoutAdjustment(gm.value(), user.cash + finalValue, cluster);
    *user.getCrypto(abbrev) -= RR::utility::round(amount, 4);
    user.mergeStat("Money Gained from " + abbrevUpper, RR::utility::curr2str(finalValue));

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::WithdrawSuccess,
        amount, abbrevUpper, RR::utility::curr2str(totalValue),
        Constants::InvestmentFeePercent, RR::utility::curr2str(finalValue)));
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
