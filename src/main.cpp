#include "data/credentials.h"
#include "database/mongomanager.h"
#include "dppcmd/services/moduleservice.h"
#include "dppinteract/interactiveservice.h"
#include "events.h"
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
#include "modules/tasks.h"
#include "modules/weapons.h"
#include "readers/cashtypereader.h"
#include "readers/rrguildmembertypereader.h"
#include "systems/monitorsystem.h"
#include <boost/locale/generator.hpp>
#include <dpp/cluster.h>
#include <sodium.h>

std::unique_ptr<dpp::cluster> cluster;
std::unique_ptr<dppinteract::interactive_service> interactive;
std::unique_ptr<dppcmd::module_service> modules;

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

    interactive = std::make_unique<dppinteract::interactive_service>();
    interactive->setup_event_handlers(cluster.get());

    dppcmd::command_service_config config { .command_prefix = '|', .throw_exceptions = true };
    modules = std::make_unique<dppcmd::module_service>(cluster.get(), config);
    modules->register_type_reader<RR::cash_in>();
    modules->register_type_reader<RR::guild_member_in>();

    modules->register_modules<Administration, BotOwner, Config>();
    modules->register_module<Crime>(interactive.get());
    modules->register_module<Economy>(interactive.get());
    modules->register_module<Fun>();
    modules->register_module<Gambling>(interactive.get());
    modules->register_modules<Gangs, General>();
    modules->register_module<Goods>(interactive.get());
    modules->register_modules<Investments, Moderation>();
    modules->register_module<Prestige>(interactive.get());
    modules->register_modules<Tasks, Weapons>();

    Events::connectEvents(cluster.get(), modules.get());
    MonitorSystem::initialize(cluster.get());

    cluster->start(dpp::st_wait);
}
