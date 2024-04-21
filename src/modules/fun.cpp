#include "fun.h"
#include "data/responses.h"
#include "utils/random.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>

Fun::Fun() : ModuleBase("Fun", "Commands that don't do anything related to the cash system and what not: they just exist for fun (hence the name).")
{
    registerCommand(&Fun::cat, "cat", "Random cat picture!");
    registerCommand(&Fun::dog, "dog", "Random dog picture!");
    registerCommand(&Fun::flip, "flip", "Flip a coin.");
    registerCommand(&Fun::gay, "gay", "See how gay you or another user is.", "$gay <user>");
    registerCommand(&Fun::godword, "godword", "God's word, sourced straight from TempleOS.", "$godword <amount>");
    registerCommand(&Fun::magicConch, "magicconch", "Ask the Magic Conch ANYTHING!", "$magicconch [question]");
    registerCommand(&Fun::penis, "penis", "See how big a user's penis is, or your own.", "$penis <user>");
    registerCommand(&Fun::sneed, "sneed", "Sneed");
    registerCommand(&Fun::terryQuote, "terryquote", "Behold a random Terry Davis quote.");
    registerCommand(&Fun::waifu, "waifu", "Get yourself a random waifu from our vast and sexy collection of scrumptious waifus.");
}

dpp::task<CommandResult> Fun::cat()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "https://api.thecatapi.com/v1/images/search", dpp::m_get);
    if (result.status != 200)
        co_return CommandResult::fromError(Responses::CatFailed);

    std::string url = nlohmann::json::parse(result.body, nullptr, false)[0]["url"].template get<std::string>();
    if (url.empty())
        co_return CommandResult::fromError(Responses::CatFailed);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::CatFound)
        .set_image(url);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return CommandResult::fromSuccess();
}

dpp::task<CommandResult> Fun::dog()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "https://dog.ceo/api/breeds/image/random", dpp::m_get);
    if (result.status != 200)
        co_return CommandResult::fromError(Responses::DogFailed);

    std::string url = nlohmann::json::parse(result.body, nullptr, false)["message"].template get<std::string>();
    if (url.empty())
        co_return CommandResult::fromError(Responses::DogFailed);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::DogFound)
        .set_image(url);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return CommandResult::fromSuccess();
}

CommandResult Fun::flip()
{
    dpp::embed embed = dpp::embed().set_color(dpp::colors::red);
    if (RR::utility::random(2) != 0)
        embed.set_title(Responses::HeadsTitle).set_image(Responses::HeadsImage);
    else
        embed.set_title(Responses::TailsTitle).set_image(Responses::TailsImage);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult Fun::gay(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);

    int gay = !user->is_bot() ? RR::utility::random(101) : 0;
    const char* title = Responses::Gay1;
    if (gay <= 10)
        title = Responses::Gay2;
    else if (gay > 10 && gay < 50)
        title = Responses::Gay3;
    else if (gay >= 50 && gay < 90)
        title = Responses::Gay4;

    std::string description = user->id == context->msg.author.id
        ? std::format(Responses::YouAreThisGay, gay) : std::format(Responses::UserIsThisGay, user->get_mention(), gay);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(title)
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

dpp::task<CommandResult> Fun::godword(const std::optional<int>& amountOpt)
{
    int amount = amountOpt ? amountOpt.value() : 1;
    dpp::http_request_completion_t result = co_await cluster->co_request(
        std::format("http://temple.xslendi.xyz/api/v1/godword?amount={}", amount), dpp::m_get);
    if (result.status != 200)
        co_return CommandResult::fromError(Responses::GodWordFailed);

    std::string words = nlohmann::json::parse(result.body, nullptr, false)["words"].template get<std::string>();
    if (words.empty())
        co_return CommandResult::fromError(Responses::GodWordFailed);
    if (words.size() > 2000)
        co_return CommandResult::fromError(Responses::GodWordTooLong);

    co_return CommandResult::fromSuccess(words);
}

CommandResult Fun::magicConch(const std::string&)
{
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::MagicConchSays)
        .set_image(RR::utility::randomElement(Responses::MagicConchImages));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult Fun::penis(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);

    int equals = !user->is_bot() ? RR::utility::random(1, 16) : 20;
    std::string title = Responses::Penis4;
    if (equals <= 3)
        title = Responses::Penis1;
    else if (equals > 3 && equals < 7)
        title = Responses::Penis2;
    else if (equals >= 7 && equals < 12)
        title = Responses::Penis3;

    std::string penis(equals, '=');
    std::string description = user->id == context->msg.author.id
        ? std::format(Responses::YourPenis, penis) : std::format(Responses::UserPenis, user->get_mention(), penis);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(title)
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult Fun::sneed()
{
    return CommandResult::fromSuccess(Responses::SneedImage);
}

dpp::task<CommandResult> Fun::terryQuote()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "http://temple.xslendi.xyz/api/v1/quote", dpp::m_get);
    if (result.status != 200)
        co_return CommandResult::fromError(Responses::TerryQuoteFailed);

    std::string quote = nlohmann::json::parse(result.body, nullptr, false)["quote"].template get<std::string>();
    if (quote.empty())
        co_return CommandResult::fromError(Responses::TerryQuoteFailed);
    if (quote.size() > 2000)
        co_return CommandResult::fromError(Responses::TerryQuoteTooLong);

    co_return CommandResult::fromSuccess("\"" + quote + "\"");
}

CommandResult Fun::waifu()
{
    auto [name, image] = RR::utility::randomElement(Responses::Waifus);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::WaifuTitle)
        .set_description(std::format(Responses::WaifuDescription, name))
        .set_image(image);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}
