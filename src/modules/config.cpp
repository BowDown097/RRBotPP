#include "config.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/entities/config/dbconfigranks.h"
#include "database/entities/config/dbconfigroles.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/moduleservice.h"
#include "dpp-command-handler/utils/join.h"
#include "dpp-command-handler/utils/strings.h"
#include "utils/ld.h"
#include <dpp/cache.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>

Config::Config() : dpp::module_base("Config", "This is where all the BORING administration stuff goes. Here, you can change how the bot does things in the server in a variety of ways. Huge generalization, but that's the best I can do.")
{
    register_command(&Config::addRank, "addrank", "Register a rank, its level, and the money required to get it.", "$addrank [level] [cost] [role]");
    register_command(&Config::clearConfig, "clearconfig", "Clear all configuration for this server.");
    register_command(&Config::currentConfig, "currentconfig", "List the current configuration for this server.");
    register_command(&Config::disableCommand, "disablecmd", "Disable a command for this server.", "$disablecmd [command]");
    register_command(&Config::disableFiltersInChannel, "disablefiltersinchannel", "Disable filters for a specific channel.", "$disablefiltersinchannel [channel]");
    register_command(&Config::disableModule, "disablemodule", "Disable a module for this server.", "$disablemodule [module]");
    register_command(&Config::enableCommand, "enablecmd", "Enable a previously disabled command.", "$enablecmd [command]");
    register_command(&Config::enableModule, "enablemodule", "Enable a previously disabled module.", "$enablemodule [module]");
    register_command(&Config::setAdminRole, "setadminrole", "Register a role that can use commands in the Administration and Config modules.", "$setadminrole [role]");
    register_command(&Config::setDjRole, "setdjrole", "Register a role as the DJ role, which is required for some of the music commands.", "$setdjrole [role]");
    register_command(&Config::setLogsChannel, "setlogschannel", "Register a channel for logs to be posted in.", "$setlogschannel [channel]");
    register_command(&Config::setModRole, "setmodrole", "Register a role that can use commands in the Moderation module.", "$setmodrole [role]");
    register_command(&Config::setPotChannel, "setpotchannel", "Register a channel for pot winnings to be announced in.", "$setpotchannel [channel]");
    register_command(&Config::toggleDrops, "toggledrops", "Toggles random drops, such as Bank Cheques.");
    register_command(&Config::toggleInviteFilter, "toggleinvitefilter", "Toggle the invite filter.");
    register_command(&Config::toggleNsfw, "togglensfw", "Enable age-restricted content to be played with the music feature.");
    register_command(&Config::toggleScamFilter, "togglescamfilter", "Toggle the scam filter.");
    register_command(&Config::unwhitelistChannel, "unwhitelistchannel", "Remove a channel from the bot command whitelist.", "$unwhitelistchannel [channel]");
    register_command(&Config::whitelistChannel, "whitelistchannel", "Add a channel to a list of whitelisted channels for bot commands. All administration, moderation, and music commands will still work in every channel.", "$whitelistchannel [channel]");
}

dpp::command_result Config::addRank(int level, long double cost, const dpp::role_in& roleIn)
{
    dpp::role* role = roleIn.top_result();

    DbConfigRanks ranks = MongoManager::fetchRankConfig(context->msg.guild_id);
    ranks.costs.emplace(level, cost);
    ranks.ids.emplace(level, role->id);

    MongoManager::updateRankConfig(ranks);
    return dpp::command_result::from_success(std::format(Responses::AddedRank, role->get_mention(), level, RR::utility::currencyToStr(cost)));
}

dpp::command_result Config::clearConfig()
{
    MongoManager::deleteChannelConfig(context->msg.guild_id);
    MongoManager::deleteMiscConfig(context->msg.guild_id);
    MongoManager::deleteRankConfig(context->msg.guild_id);
    MongoManager::deleteRoleConfig(context->msg.guild_id);
    return dpp::command_result::from_success(Responses::ClearedConfig);
}

dpp::command_result Config::currentConfig()
{
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    auto noFilterChannels = channels.noFilterChannels | std::views::transform([](int64_t id) {
        return dpp::channel::get_mention(id);
    });
    auto whitelisted = channels.whitelistedChannels | std::views::transform([](int64_t id) {
        return dpp::channel::get_mention(id);
    });

    std::string description = "***Channels***\n";
    description += std::format("Command Whitelisted Channels: {}\n", dpp::utility::join(whitelisted, ", "));
    description += std::format("Logs Channel: {}\n", dpp::channel::get_mention(channels.logsChannel));
    description += std::format("No Filter Channels: {}\n", dpp::utility::join(noFilterChannels, ", "));
    description += std::format("Pot Channel: {}\n", dpp::channel::get_mention(channels.potChannel));

    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    description += "***Miscellaneous***\n";
    description += std::format("Disabled Commands: {}\n", dpp::utility::join(misc.disabledCommands, ", "));
    description += std::format("Disabled Modules: {}\n", dpp::utility::join(misc.disabledModules, ", "));
    description += std::format("Invite Filter Enabled: {}\n", misc.inviteFilterEnabled);
    description += std::format("NSFW Enabled: {}\n", misc.nsfwEnabled);
    description += std::format("Scam Filter Enabled: {}\n", misc.scamFilterEnabled);

    DbConfigRanks ranks = MongoManager::fetchRankConfig(context->msg.guild_id);
    description += "***Ranks***\n";
    if (!ranks.costs.empty())
    {
        std::map<int, long double> costs(std::make_move_iterator(ranks.costs.begin()),
                                         std::make_move_iterator(ranks.costs.end()));
        for (const auto& [level, cost] : costs)
        {
            description += std::format("Level {}: {} - {}\n",
                level, dpp::role::get_mention(ranks.ids[level]), RR::utility::currencyToStr(cost));
        }
    }
    else
    {
        description += "None\n";
    }

    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    description += "***Roles***\n";
    description += std::format("Admin Role: {}\n", dpp::role::get_mention(roles.staffLvl2Role));
    description += std::format("DJ Role: {}\n", dpp::role::get_mention(roles.djRole));
    description += std::format("Moderator Role: {}", dpp::role::get_mention(roles.staffLvl1Role));

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Current Configuration")
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result Config::disableCommand(const std::string& cmd)
{
    if (dpp::utility::iequals(cmd, "disablecmd") || dpp::utility::iequals(cmd, "enablecmd"))
        return dpp::command_result::from_error(Responses::BadIdea);

    std::vector<std::reference_wrapper<const dpp::command_info>> commands = service->search_command(cmd);
    if (commands.empty())
        return dpp::command_result::from_error(Responses::NonexistentCommand);

    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.disabledCommands.push_back(commands[0].get().name());

    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(Responses::SetCommandDisabled);
}

dpp::command_result Config::disableFiltersInChannel(const dpp::channel_in& channelIn)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!misc.inviteFilterEnabled && !misc.scamFilterEnabled)
        return dpp::command_result::from_error(Responses::NoFiltersToDisable);

    dpp::channel* channel = channelIn.top_result();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.noFilterChannels.push_back(channel->id);

    MongoManager::updateChannelConfig(channels);
    return dpp::command_result::from_success(std::format(Responses::DisabledFilters, channel->get_mention()));
}

dpp::command_result Config::disableModule(const std::string& module)
{
    if (dpp::utility::iequals(module, "Config"))
        return dpp::command_result::from_error(Responses::BadIdea);

    std::vector<std::reference_wrapper<const dpp::module_base>> modules = service->search_module(module);
    if (modules.empty())
        return dpp::command_result::from_error(Responses::NonexistentModule);

    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.disabledModules.push_back(modules[0].get().name());

    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(Responses::SetModuleDisabled);
}

dpp::command_result Config::enableCommand(const std::string& cmd)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!std::erase_if(misc.disabledCommands, [&cmd](const std::string& c) { return dpp::utility::iequals(c, cmd); }))
        return dpp::command_result::from_error(Responses::NotDisabledCommand);

    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(Responses::SetCommandEnabled);
}

dpp::command_result Config::enableModule(const std::string& module)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!std::erase_if(misc.disabledModules, [&module](const std::string& m) { return dpp::utility::iequals(m, module); }))
        return dpp::command_result::from_error(Responses::NotDisabledModule);

    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(Responses::SetModuleEnabled);
}

dpp::command_result Config::setAdminRole(const dpp::role_in& roleIn)
{
    dpp::role* role = roleIn.top_result();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.staffLvl2Role = role->id;
    MongoManager::updateRoleConfig(roles);
    return dpp::command_result::from_success(std::format(Responses::SetAdminRole, role->get_mention()));
}

dpp::command_result Config::setDjRole(const dpp::role_in& roleIn)
{
    dpp::role* role = roleIn.top_result();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.djRole = role->id;
    MongoManager::updateRoleConfig(roles);
    return dpp::command_result::from_success(std::format(Responses::SetDjRole, role->get_mention()));
}

dpp::command_result Config::setLogsChannel(const dpp::channel_in& channelIn)
{
    dpp::channel* channel = channelIn.top_result();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.logsChannel = channel->id;
    MongoManager::updateChannelConfig(channels);
    return dpp::command_result::from_success(std::format(Responses::SetLogsChannel, channel->get_mention()));
}

dpp::command_result Config::setModRole(const dpp::role_in& roleIn)
{
    dpp::role* role = roleIn.top_result();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.staffLvl1Role = role->id;
    MongoManager::updateRoleConfig(roles);
    return dpp::command_result::from_success(std::format(Responses::SetModRole, role->get_mention()));
}

dpp::command_result Config::setPotChannel(const dpp::channel_in& channelIn)
{
    dpp::channel* channel = channelIn.top_result();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.potChannel = channel->id;
    MongoManager::updateChannelConfig(channels);
    return dpp::command_result::from_success(std::format(Responses::SetPotChannel, channel->get_mention()));
}

dpp::command_result Config::toggleDrops()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.dropsDisabled = !misc.dropsDisabled;
    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(std::format(Responses::ToggledRandomDrops, misc.dropsDisabled ? "OFF" : "ON"));
}

dpp::command_result Config::toggleInviteFilter()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.inviteFilterEnabled = !misc.inviteFilterEnabled;
    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(std::format(Responses::ToggledInviteFilter, misc.inviteFilterEnabled ? "ON" : "OFF"));
}

dpp::command_result Config::toggleNsfw()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.nsfwEnabled = !misc.nsfwEnabled;
    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(std::format(Responses::ToggledNsfw, misc.nsfwEnabled ? "ON" : "OFF"));
}

dpp::command_result Config::toggleScamFilter()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.scamFilterEnabled = !misc.scamFilterEnabled;
    MongoManager::updateMiscConfig(misc);
    return dpp::command_result::from_success(std::format(Responses::ToggledScamFilter, misc.scamFilterEnabled ? "ON" : "OFF"));
}

dpp::command_result Config::unwhitelistChannel(const dpp::channel_in& channelIn)
{
    dpp::channel* channel = channelIn.top_result();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    if (!std::erase(channels.whitelistedChannels, channel->id))
        return dpp::command_result::from_error(Responses::ChannelNotWhitelisted);

    MongoManager::updateChannelConfig(channels);
    return dpp::command_result::from_success(std::format(Responses::ChannelUnwhitelisted, channel->get_mention()));
}

dpp::command_result Config::whitelistChannel(const dpp::channel_in& channelIn)
{
    dpp::channel* channel = channelIn.top_result();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.whitelistedChannels.push_back(channel->id);
    MongoManager::updateChannelConfig(channels);
    return dpp::command_result::from_success(std::format(Responses::ChannelWhitelisted, channel->get_mention()));
}
