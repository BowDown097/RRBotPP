#include "botowner.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/services/moduleservice.h"
#include "dpp-command-handler/utils/strings.h"
#include <dpp/dispatcher.h>
#include <format>

BotOwner::BotOwner() : dpp::module<BotOwner>("BotOwner", "Commands for bot owners only.")
{
    register_command(&BotOwner::blacklist, "blacklist", "Ban a user from using the bot.", "$blacklist [user]");
    register_command(&BotOwner::disableCommandGlobal, "disablecmdglobal", "Globally disable a command.", "$disablecmdglobal [command]");
    register_command(&BotOwner::enableCommandGlobal, "enablecmdglobal", "Globally enable a previously disabled command.", "$enablecmdglobal [command]");
    register_command(&BotOwner::resetUser, "resetuser", "Completely reset a user.", "$resetuser [user]");
    register_command(&BotOwner::unblacklist, "unblacklist", "Unban a user from using the bot." "$unblacklist [user]");
}

dpp::command_result BotOwner::blacklist(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        return dpp::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.bannedUsers.insert(member.user_id);

    MongoManager::updateGlobalConfig(globalConfig);
    return dpp::command_result::from_success(std::format(Responses::SetUserBlacklisted, member.get_mention()));
}

dpp::command_result BotOwner::disableCommandGlobal(const std::string& cmd)
{
    if (dpp::utility::iequals(cmd, "disablecmdglobal") || dpp::utility::iequals(cmd, "enablecmdglobal"))
        return dpp::command_result::from_error(Responses::BadIdea);

    std::vector<std::reference_wrapper<const dpp::command_info>> commands = service->search_command(cmd);
    if (commands.empty())
        return dpp::command_result::from_error(Responses::NonexistentCommand);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.disabledCommands.insert(commands[0].get().name());

    MongoManager::updateGlobalConfig(globalConfig);
    return dpp::command_result::from_success(Responses::SetCommandDisabled);
}

dpp::command_result BotOwner::enableCommandGlobal(const std::string& cmd)
{
    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!std::erase_if(globalConfig.disabledCommands, [&cmd](const std::string& c) { return dpp::utility::iequals(c, cmd); }))
        return dpp::command_result::from_error(Responses::NotDisabledCommand);

    MongoManager::updateGlobalConfig(globalConfig);
    return dpp::command_result::from_success(Responses::SetCommandEnabled);
}

dpp::command_result BotOwner::resetUser(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    MongoManager::deleteUser(member.user_id, context->msg.guild_id);
    return dpp::command_result::from_success(std::format(Responses::ResetUser, member.get_mention()));
}

dpp::command_result BotOwner::unblacklist(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!globalConfig.bannedUsers.erase(member.user_id))
        return dpp::command_result::from_error(std::format(Responses::UserNotBlacklisted, member.get_mention()));

    MongoManager::updateGlobalConfig(globalConfig);
    return dpp::command_result::from_success(std::format(Responses::SetUserUnblacklisted, member.get_mention()));
}
