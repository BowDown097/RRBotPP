#include "data/credentials.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/moduleservice.h"
#include "modules/administration.h"
#include "modules/botowner.h"
#include "modules/config.h"
#include "modules/crime.h"
#include "modules/economy.h"
#include "modules/fun.h"
#include "modules/general.h"
#include <boost/locale/generator.hpp>
#include <dpp/cluster.h>
#include <sodium.h>

std::unique_ptr<ModuleService> moduleService;

dpp::task<void> handleMessage(const dpp::message_create_t& event)
{
    CommandResult result = co_await moduleService->handleMessage(&event);
    if (result.message().empty())
        co_return;

    std::optional<CommandError> error = result.error();
    if (result.success() || error == CommandError::Unsuccessful || error == CommandError::UnmetPrecondition)
    {
        event.reply(result.message());
    }
    else if (error == CommandError::Exception || error == CommandError::ObjectNotFound ||
             error == CommandError::ParseFailed || error == CommandError::BadArgCount)
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

    moduleService = std::make_unique<ModuleService>(client.get(), ModuleServiceConfig { .commandPrefix = '|' });
    moduleService->registerModules<Administration, BotOwner, Config, Crime, Economy, Fun, General>();

    client->on_log(dpp::utility::cout_logger());
    client->on_message_create(&handleMessage);

    client->start(dpp::st_wait);
}
