#include "botowner.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/mongomanager.h"
#include "dppcmd/services/moduleservice.h"
#include "dppcmd/utils/strings.h"
#include <dpp/dispatcher.h>
#include <format>

BotOwner::BotOwner() : dppcmd::module<BotOwner>("BotOwner", "Commands for bot owners only.")
{
    register_command(&BotOwner::blacklist, "blacklist", "Ban a user from using the bot.", "$blacklist [user]");
    register_command(&BotOwner::disableCommandGlobal, "disablecmdglobal", "Globally disable a command.", "$disablecmdglobal [command]");
    register_command(&BotOwner::enableCommandGlobal, "enablecmdglobal", "Globally enable a previously disabled command.", "$enablecmdglobal [command]");
    register_command(&BotOwner::resetUser, "resetuser", "Completely reset a user.", "$resetuser [user]");
    register_command(&BotOwner::unblacklist, "unblacklist", "Unban a user from using the bot." "$unblacklist [user]");
}

dppcmd::command_result BotOwner::blacklist(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.bannedUsers.insert(member.user_id);

    MongoManager::updateGlobalConfig(globalConfig);
    return dppcmd::command_result::from_success(std::format(Responses::SetUserBlacklisted, member.get_mention()));
}

dppcmd::command_result BotOwner::disableCommandGlobal(const std::string& cmd)
{
    if (dppcmd::utility::iequals(cmd, "disablecmdglobal") || dppcmd::utility::iequals(cmd, "enablecmdglobal"))
        return dppcmd::command_result::from_error(Responses::BadIdea);

    std::vector<const dppcmd::command_info*> cmds = service->search_command(cmd);
    if (cmds.empty())
        return dppcmd::command_result::from_error(Responses::NonexistentCommand);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    globalConfig.disabledCommands.insert(cmds.front()->name());

    MongoManager::updateGlobalConfig(globalConfig);
    return dppcmd::command_result::from_success(Responses::SetCommandDisabled);
}

dppcmd::command_result BotOwner::enableCommandGlobal(const std::string& cmd)
{
    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!std::erase_if(globalConfig.disabledCommands, [&cmd](const std::string& c) { return dppcmd::utility::iequals(c, cmd); }))
        return dppcmd::command_result::from_error(Responses::NotDisabledCommand);

    MongoManager::updateGlobalConfig(globalConfig);
    return dppcmd::command_result::from_success(Responses::SetCommandEnabled);
}

dppcmd::command_result BotOwner::resetUser(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    MongoManager::deleteUser(member.user_id, context->msg.guild_id);
    return dppcmd::command_result::from_success(std::format(Responses::ResetUser, member.get_mention()));
}

dppcmd::command_result BotOwner::unblacklist(const dpp::guild_member& member)
{
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
    if (!globalConfig.bannedUsers.erase(member.user_id))
        return dppcmd::command_result::from_error(std::format(Responses::UserNotBlacklisted, member.get_mention()));

    MongoManager::updateGlobalConfig(globalConfig);
    return dppcmd::command_result::from_success(std::format(Responses::SetUserUnblacklisted, member.get_mention()));
}
