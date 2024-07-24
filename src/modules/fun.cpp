#include "fun.h"
#include "data/responses.h"
#include "utils/random.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>

Fun::Fun() : dpp::module<Fun>("Fun", "Commands that don't do anything related to the cash system and what not: they just exist for fun (hence the name).")
{
    register_command(&Fun::cat, "cat", "Random cat picture!");
    register_command(&Fun::dog, "dog", "Random dog picture!");
    register_command(&Fun::flip, "flip", "Flip a coin.");
    register_command(&Fun::gay, "gay", "See how gay you or another user is.", "$gay <user>");
    register_command(&Fun::godword, "godword", "God's word, sourced straight from TempleOS.", "$godword <amount>");
    register_command(&Fun::magicConch, "magicconch", "Ask the Magic Conch ANYTHING!", "$magicconch [question]");
    register_command(&Fun::penis, "penis", "See how big a user's penis is, or your own.", "$penis <user>");
    register_command(&Fun::sneed, "sneed", "Sneed");
    register_command(&Fun::terryQuote, "terryquote", "Behold a random Terry Davis quote.");
    register_command(&Fun::waifu, "waifu", "Get yourself a random waifu from our vast and sexy collection of scrumptious waifus.");
}

dpp::task<dpp::command_result> Fun::cat()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "https://api.thecatapi.com/v1/images/search", dpp::m_get);
    if (result.status != 200)
        co_return dpp::command_result::from_error(Responses::CatFailed);

    std::string url = nlohmann::json::parse(result.body, nullptr, false)[0]["url"].template get<std::string>();
    if (url.empty())
        co_return dpp::command_result::from_error(Responses::CatFailed);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::CatFound)
        .set_image(url);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Fun::dog()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "https://dog.ceo/api/breeds/image/random", dpp::m_get);
    if (result.status != 200)
        co_return dpp::command_result::from_error(Responses::DogFailed);

    std::string url = nlohmann::json::parse(result.body, nullptr, false)["message"].template get<std::string>();
    if (url.empty())
        co_return dpp::command_result::from_error(Responses::DogFailed);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::DogFound)
        .set_image(url);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return dpp::command_result::from_success();
}

dpp::command_result Fun::flip()
{
    dpp::embed embed = dpp::embed().set_color(dpp::colors::red);
    if (RR::utility::random(2) != 0)
        embed.set_title(Responses::HeadsTitle).set_image(Responses::HeadsImage);
    else
        embed.set_title(Responses::TailsTitle).set_image(Responses::TailsImage);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result Fun::gay(const std::optional<dpp::user_in>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->top_result() : &context->msg.author;
    if (!user)
        return dpp::command_result::from_error(Responses::GetUserFailed);

    int gay = !user->is_bot() ? RR::utility::random(101) : 0;
    const char* title = Responses::Gay1;
    if (gay <= 10)
        title = Responses::Gay2;
    else if (gay < 50)
        title = Responses::Gay3;
    else if (gay < 90)
        title = Responses::Gay4;

    std::string description = user->id == context->msg.author.id
        ? std::format(Responses::YouAreThisGay, gay) : std::format(Responses::UserIsThisGay, user->get_mention(), gay);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(title)
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> Fun::godword(const std::optional<int>& amountOpt)
{
    int amount = amountOpt ? amountOpt.value() : 1;
    dpp::http_request_completion_t result = co_await cluster->co_request(
        std::format("http://temple.xslendi.xyz/api/v1/godword?amount={}", amount), dpp::m_get);
    if (result.status != 200)
        co_return dpp::command_result::from_error(Responses::GodWordFailed);

    std::string words = nlohmann::json::parse(result.body, nullptr, false)["words"].template get<std::string>();
    if (words.empty())
        co_return dpp::command_result::from_error(Responses::GodWordFailed);
    if (words.size() > 2000)
        co_return dpp::command_result::from_error(Responses::GodWordTooLong);

    co_return dpp::command_result::from_success(words);
}

dpp::command_result Fun::magicConch(const dpp::remainder<std::string>&)
{
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::MagicConchSays)
        .set_image(RR::utility::randomElement(Responses::MagicConchImages));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result Fun::penis(const std::optional<dpp::user_in>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->top_result() : &context->msg.author;
    if (!user)
        return dpp::command_result::from_error(Responses::GetUserFailed);

    int equals = !user->is_bot() ? RR::utility::random(1, 16) : 20;
    std::string title = Responses::Penis4;
    if (equals <= 3)
        title = Responses::Penis1;
    else if (equals < 7)
        title = Responses::Penis2;
    else if (equals < 12)
        title = Responses::Penis3;

    std::string penis(equals, '=');
    std::string description = user->id == context->msg.author.id
        ? std::format(Responses::YourPenis, penis) : std::format(Responses::UserPenis, user->get_mention(), penis);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(title)
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result Fun::sneed()
{
    return dpp::command_result::from_success(Responses::SneedImage);
}

dpp::task<dpp::command_result> Fun::terryQuote()
{
    dpp::http_request_completion_t result = co_await cluster->co_request(
        "http://temple.xslendi.xyz/api/v1/quote", dpp::m_get);
    if (result.status != 200)
        co_return dpp::command_result::from_error(Responses::TerryQuoteFailed);

    std::string quote = nlohmann::json::parse(result.body, nullptr, false)["quote"].template get<std::string>();
    if (quote.empty())
        co_return dpp::command_result::from_error(Responses::TerryQuoteFailed);
    if (quote.size() > 2000)
        co_return dpp::command_result::from_error(Responses::TerryQuoteTooLong);

    co_return dpp::command_result::from_success("\"" + quote + "\"");
}

dpp::command_result Fun::waifu()
{
    auto [name, image] = RR::utility::randomElement(Responses::Waifus);
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(Responses::WaifuTitle)
        .set_description(std::format(Responses::WaifuDescription, name))
        .set_image(image);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}
