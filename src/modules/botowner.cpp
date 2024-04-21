#include "botowner.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/moduleservice.h"
#include "dpp-command-handler/utils/strings.h"
#include <dpp/dispatcher.h>
#include <format>

BotOwner::BotOwner() : ModuleBase("BotOwner", "Commands for bot owners only.")
{
    registerCommand(&BotOwner::blacklist, "blacklist", "Ban a user from using the bot.", "$blacklist [user]");
    registerCommand(&BotOwner::disableCommandGlobal, "disablecmdglobal", "Globally disable a command.", "$disablecmdglobal [command]");
    registerCommand(&BotOwner::enableCommandGlobal, "enablecmdglobal", "Globally enable a previously disabled command.", "$enablecmdglobal [command]");
    registerCommand(&BotOwner::resetUser, "resetuser", "Completely reset a user.", "$resetuser [user]");
    registerCommand(&BotOwner::unblacklist, "unblacklist", "Unban a user from using the bot." "$unblacklist [user]");
}

CommandResult BotOwner::blacklist(const UserTypeReader& userRead)
{
    dpp::user* user = userRead.topResult();
    if (user->id == context->msg.author.id)
        return CommandResult::fromError(Responses::BadIdea);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.bannedUsers.push_back(user->id);

    MongoManager::updateGlobalConfig(globalConfig);
    return CommandResult::fromSuccess(std::format(Responses::SetUserBlacklisted, user->get_mention()));
}

CommandResult BotOwner::disableCommandGlobal(const std::string& cmd)
{
    if (dpp::utility::iequals(cmd, "disablecmdglobal") || dpp::utility::iequals(cmd, "enablecmdglobal"))
        return CommandResult::fromError(Responses::BadIdea);

    std::vector<std::reference_wrapper<const CommandInfo>> commands = service->searchCommand(cmd);
    if (commands.empty())
        return CommandResult::fromError(Responses::NonexistentCommand);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.disabledCommands.push_back(commands[0].get().name());

    MongoManager::updateGlobalConfig(globalConfig);
    return CommandResult::fromSuccess(Responses::SetCommandDisabled);
}

CommandResult BotOwner::enableCommandGlobal(const std::string& cmd)
{
    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!std::erase_if(globalConfig.disabledCommands, [&cmd](const std::string& c) { return dpp::utility::iequals(c, cmd); }))
        return CommandResult::fromError(Responses::NotDisabledCommand);

    MongoManager::updateGlobalConfig(globalConfig);
    return CommandResult::fromSuccess(Responses::SetCommandEnabled);
}

CommandResult BotOwner::resetUser(const UserTypeReader& userRead)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    MongoManager::deleteUser(user->id, context->msg.guild_id);
    return CommandResult::fromSuccess(std::format(Responses::ResetUser, user->get_mention()));
}

CommandResult BotOwner::unblacklist(const UserTypeReader& userRead)
{
    dpp::user* user = userRead.topResult();
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!std::erase(globalConfig.bannedUsers, user->id))
        return CommandResult::fromError(std::format(Responses::UserNotBlacklisted, user->get_mention()));

    MongoManager::updateGlobalConfig(globalConfig);
    return CommandResult::fromSuccess(std::format(Responses::SetUserUnblacklisted, user->get_mention()));
}
