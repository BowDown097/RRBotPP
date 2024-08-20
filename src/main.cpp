#include "data/credentials.h"
#include "data/responses.h"
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
#include "modules/moderation.h"
#include "modules/prestige.h"
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

    try
    {
        dpp::command_result result = co_await modules->handle_message(&event);
        if (result.message().empty())
            co_return;

        std::optional<dpp::command_error> error = result.error();
        if (result.success() || error == dpp::command_error::unsuccessful || error == dpp::command_error::unmet_precondition)
            event.reply(result.message());
    }
    catch (const dpp::bad_argument_count& ex)
    {
        std::vector<std::reference_wrapper<const dpp::command_info>> cmds = modules->search_command(ex.command());
        event.reply(!cmds.empty()
            ? std::format(Responses::BadArgCount, ex.target_arg_count(), cmds.front().get().remarks())
            : std::format(Responses::ErrorOccurred, ex.what()));
    }
    catch (const dpp::bad_command_argument& ex)
    {
        if (ex.error() == dpp::command_error::multiple_matches)
        {
            event.reply(ex.message());
            co_return;
        }

        std::vector<std::reference_wrapper<const dpp::command_info>> cmds = modules->search_command(ex.command());
        event.reply(!cmds.empty()
            ? std::format(Responses::BadArgument, ex.arg(), ex.message(), cmds.front().get().remarks())
            : std::format(Responses::ErrorOccurred, ex.what()));
    }
    catch (const std::exception& ex)
    {
        event.reply(std::format(Responses::ErrorOccurred, ex.what()));
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

    cluster = std::make_unique<dpp::cluster>(
        Credentials::instance().token(),
        dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members
    );

    interactive = std::make_unique<dpp::interactive_service>();
    interactive->setup_event_handlers(cluster.get());

    dpp::command_service_config config { .command_prefix = '|', .throw_exceptions = true };
    modules = std::make_unique<dpp::module_service>(cluster.get(), config);
    modules->register_modules<Administration, BotOwner, Config>();
    modules->register_module<Crime>(interactive.get());
    modules->register_module<Economy>(interactive.get());
    modules->register_module<Fun>();
    modules->register_module<Gambling>(interactive.get());
    modules->register_modules<Gangs, General>();
    modules->register_module<Goods>(interactive.get());
    modules->register_modules<Investments, Moderation>();
    modules->register_module<Prestige>(interactive.get());

    cluster->on_log(dpp::utility::cout_logger());
    cluster->on_message_create(&handleMessage);

    MonitorSystem::initialize(cluster.get());

    cluster->start(dpp::st_wait);
}
