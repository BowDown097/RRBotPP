#include "data/credentials.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/services/moduleservice.h"
#include "dpp-interactive/interactiveservice.h"
#include "modules/administration.h"
#include "modules/botowner.h"
#include "modules/config.h"
#include "modules/crime.h"
#include "modules/economy.h"
#include "modules/fun.h"
#include "modules/gambling.h"
#include "modules/gangs.h"
#include "modules/general.h"
#include "modules/goods.h"
#include "modules/investments.h"
#include "systems/filtersystem.h"
#include "systems/monitorsystem.h"
#include <boost/locale/generator.hpp>
#include <dpp/cluster.h>
#include <sodium.h>

std::unique_ptr<dpp::cluster> cluster;
std::unique_ptr<dpp::interactive_service> interactive;
std::unique_ptr<dpp::module_service> modules;

dpp::task<void> handleMessage(const dpp::message_create_t& event)
{
    if (event.msg.content.empty() || event.msg.author.is_bot())
        co_return;

    co_await FilterSystem::doFilteredWordCheck(event.msg, cluster.get());
    co_await FilterSystem::doInviteCheck(event.msg, cluster.get());
    co_await FilterSystem::doScamCheck(event.msg, cluster.get());

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

    co_return;
}

dpp::task<void> onButtonClick(const dpp::button_click_t& event)
{
    co_await interactive->handle_button_click(event);
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

    cluster = std::make_unique<dpp::cluster>(
        Credentials::instance().token(),
        dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members
    );

    interactive = std::make_unique<dpp::interactive_service>(cluster.get());

    modules = std::make_unique<dpp::module_service>(cluster.get(), dpp::command_service_config { .command_prefix = '|' });
    modules->register_modules<Administration, BotOwner, Config, Crime, Economy, Fun, Gambling, Gangs, General>();
    modules->register_module<Goods>(interactive.get());
    modules->register_module<Investments>();

    cluster->on_button_click(&onButtonClick);
    cluster->on_log(dpp::utility::cout_logger());
    cluster->on_message_create(&handleMessage);

    MonitorSystem::initialize(cluster.get());

    cluster->start(dpp::st_wait);
}
