#include "data/credentials.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/moduleservice.h"
#include "modules/administration.h"
#include "modules/botowner.h"
#include "modules/config.h"
#include "modules/crime.h"
#include "modules/economy.h"
#include "modules/fun.h"
#include "modules/gambling.h"
#include "modules/gangs.h"
#include "modules/general.h"
#include <boost/locale/generator.hpp>
#include <dpp/cluster.h>
#include <sodium.h>

std::unique_ptr<dpp::module_service> modules;

dpp::task<void> handleMessage(const dpp::message_create_t& event)
{
    dpp::command_result result = co_await modules->handle_message(&event);
    if (result.message().empty())
        co_return;

    std::optional<dpp::command_error> error = result.error();
    if (result.success() || error == dpp::command_error::unsuccessful || error == dpp::command_error::unmet_precondition)
    {
        event.reply(result.message());
    }
    else if (error == dpp::command_error::exception || error == dpp::command_error::object_not_found ||
             error == dpp::command_error::parse_failed || error == dpp::command_error::bad_arg_count)
    {
        std::cout << dpp::utility::lexical_cast<std::string>(error.value()) << ": " << result.message() << std::endl;
    }
}

int main()
{
    Credentials::instance().initialize();
    MongoManager::initialize(Credentials::instance().mongoConnectionString());
    std::locale::global(boost::locale::generator()(""));

    if (sodium_init() == -1)
    {
        std::cerr << "Failed to initialize libsodium." << std::endl;
        return EXIT_FAILURE;
    }

    auto client = std::make_unique<dpp::cluster>(
        Credentials::instance().token(),
        dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members
    );

    modules = std::make_unique<dpp::module_service>(client.get(), dpp::module_service_config { .command_prefix = '|' });
    modules->register_modules<Administration, BotOwner, Config, Crime, Economy, Fun, Gambling, Gangs, General>();

    client->on_log(dpp::utility::cout_logger());
    client->on_message_create(&handleMessage);

    client->start(dpp::st_wait);
}
