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

Config::Config() : ModuleBase("Config", "This is where all the BORING administration stuff goes. Here, you can change how the bot does things in the server in a variety of ways. Huge generalization, but that's the best I can do.")
{
    registerCommand(&Config::addRank, "addrank", "Register a rank, its level, and the money required to get it.", "$addrank [level] [cost] [role]");
    registerCommand(&Config::clearConfig, "clearconfig", "Clear all configuration for this server.");
    registerCommand(&Config::currentConfig, "currentconfig", "List the current configuration for this server.");
    registerCommand(&Config::disableCommand, "disablecmd", "Disable a command for this server.", "$disablecmd [command]");
    registerCommand(&Config::disableFiltersInChannel, "disablefiltersinchannel", "Disable filters for a specific channel.", "$disablefiltersinchannel [channel]");
    registerCommand(&Config::disableModule, "disablemodule", "Disable a module for this server.", "$disablemodule [module]");
    registerCommand(&Config::enableCommand, "enablecmd", "Enable a previously disabled command.", "$enablecmd [command]");
    registerCommand(&Config::enableModule, "enablemodule", "Enable a previously disabled module.", "$enablemodule [module]");
    registerCommand(&Config::setAdminRole, "setadminrole", "Register a role that can use commands in the Administration and Config modules.", "$setadminrole [role]");
    registerCommand(&Config::setDjRole, "setdjrole", "Register a role as the DJ role, which is required for some of the music commands.", "$setdjrole [role]");
    registerCommand(&Config::setLogsChannel, "setlogschannel", "Register a channel for logs to be posted in.", "$setlogschannel [channel]");
    registerCommand(&Config::setModRole, "setmodrole", "Register a role that can use commands in the Moderation module.", "$setmodrole [role]");
    registerCommand(&Config::setPotChannel, "setpotchannel", "Register a channel for pot winnings to be announced in.", "$setpotchannel [channel]");
    registerCommand(&Config::toggleDrops, "toggledrops", "Toggles random drops, such as Bank Cheques.");
    registerCommand(&Config::toggleInviteFilter, "toggleinvitefilter", "Toggle the invite filter.");
    registerCommand(&Config::toggleNsfw, "togglensfw", "Enable age-restricted content to be played with the music feature.");
    registerCommand(&Config::toggleScamFilter, "togglescamfilter", "Toggle the scam filter.");
    registerCommand(&Config::unwhitelistChannel, "unwhitelistchannel", "Remove a channel from the bot command whitelist.", "$unwhitelistchannel [channel]");
    registerCommand(&Config::whitelistChannel, "whitelistchannel", "Add a channel to a list of whitelisted channels for bot commands. All administration, moderation, and music commands will still work in every channel.", "$whitelistchannel [channel]");
}

CommandResult Config::addRank(int level, long double cost, const RoleTypeReader& roleRead)
{
    dpp::role* role = roleRead.topResult();

    DbConfigRanks ranks = MongoManager::fetchRankConfig(context->msg.guild_id);
    ranks.costs.emplace(level, cost);
    ranks.ids.emplace(level, role->id);

    MongoManager::updateRankConfig(ranks);
    return CommandResult::fromSuccess(std::format(Responses::AddedRank, role->get_mention(), level, RR::utility::currencyToStr(cost)));
}

CommandResult Config::clearConfig()
{
    MongoManager::deleteChannelConfig(context->msg.guild_id);
    MongoManager::deleteMiscConfig(context->msg.guild_id);
    MongoManager::deleteRankConfig(context->msg.guild_id);
    MongoManager::deleteRoleConfig(context->msg.guild_id);
    return CommandResult::fromSuccess(Responses::ClearedConfig);
}

CommandResult Config::currentConfig()
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
    return CommandResult::fromSuccess();
}

CommandResult Config::disableCommand(const std::string& cmd)
{
    if (dpp::utility::iequals(cmd, "disablecmd") || dpp::utility::iequals(cmd, "enablecmd"))
        return CommandResult::fromError(Responses::BadIdea);

    std::vector<std::reference_wrapper<const CommandInfo>> commands = service->searchCommand(cmd);
    if (commands.empty())
        return CommandResult::fromError(Responses::NonexistentCommand);

    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.disabledCommands.push_back(commands[0].get().name());

    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(Responses::SetCommandDisabled);
}

CommandResult Config::disableFiltersInChannel(const ChannelTypeReader& channelRead)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!misc.inviteFilterEnabled && !misc.scamFilterEnabled)
        return CommandResult::fromError(Responses::NoFiltersToDisable);

    dpp::channel* channel = channelRead.topResult();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.noFilterChannels.push_back(channelRead.topResult()->id);

    MongoManager::updateChannelConfig(channels);
    return CommandResult::fromSuccess(std::format(Responses::DisabledFilters, channel->get_mention()));
}

CommandResult Config::disableModule(const std::string& module)
{
    if (dpp::utility::iequals(module, "Config"))
        return CommandResult::fromError(Responses::BadIdea);

    std::vector<std::reference_wrapper<const ModuleBase>> modules = service->searchModule(module);
    if (modules.empty())
        return CommandResult::fromError(Responses::NonexistentModule);

    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.disabledModules.push_back(modules[0].get().name());

    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(Responses::SetModuleDisabled);
}

CommandResult Config::enableCommand(const std::string& cmd)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!std::erase_if(misc.disabledCommands, [&cmd](const std::string& c) { return dpp::utility::iequals(c, cmd); }))
        return CommandResult::fromError(Responses::NotDisabledCommand);

    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(Responses::SetCommandEnabled);
}

CommandResult Config::enableModule(const std::string& module)
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    if (!std::erase_if(misc.disabledModules, [&module](const std::string& m) { return dpp::utility::iequals(m, module); }))
        return CommandResult::fromError(Responses::NotDisabledModule);

    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(Responses::SetModuleEnabled);
}

CommandResult Config::setAdminRole(const RoleTypeReader& roleRead)
{
    dpp::role* role = roleRead.topResult();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.staffLvl2Role = role->id;
    MongoManager::updateRoleConfig(roles);
    return CommandResult::fromSuccess(std::format(Responses::SetAdminRole, role->get_mention()));
}

CommandResult Config::setDjRole(const RoleTypeReader& roleRead)
{
    dpp::role* role = roleRead.topResult();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.djRole = role->id;
    MongoManager::updateRoleConfig(roles);
    return CommandResult::fromSuccess(std::format(Responses::SetDjRole, role->get_mention()));
}

CommandResult Config::setLogsChannel(const ChannelTypeReader& channelRead)
{
    dpp::channel* channel = channelRead.topResult();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.logsChannel = channel->id;
    MongoManager::updateChannelConfig(channels);
    return CommandResult::fromSuccess(std::format(Responses::SetLogsChannel, channel->get_mention()));
}

CommandResult Config::setModRole(const RoleTypeReader& roleRead)
{
    dpp::role* role = roleRead.topResult();
    DbConfigRoles roles = MongoManager::fetchRoleConfig(context->msg.guild_id);
    roles.staffLvl1Role = role->id;
    MongoManager::updateRoleConfig(roles);
    return CommandResult::fromSuccess(std::format(Responses::SetModRole, role->get_mention()));
}

CommandResult Config::setPotChannel(const ChannelTypeReader& channelRead)
{
    dpp::channel* channel = channelRead.topResult();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.potChannel = channel->id;
    MongoManager::updateChannelConfig(channels);
    return CommandResult::fromSuccess(std::format(Responses::SetPotChannel, channel->get_mention()));
}

CommandResult Config::toggleDrops()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.dropsDisabled = !misc.dropsDisabled;
    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(std::format(Responses::ToggledRandomDrops, misc.dropsDisabled ? "OFF" : "ON"));
}

CommandResult Config::toggleInviteFilter()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.inviteFilterEnabled = !misc.inviteFilterEnabled;
    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(std::format(Responses::ToggledInviteFilter, misc.inviteFilterEnabled ? "ON" : "OFF"));
}

CommandResult Config::toggleNsfw()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.nsfwEnabled = !misc.nsfwEnabled;
    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(std::format(Responses::ToggledNsfw, misc.nsfwEnabled ? "ON" : "OFF"));
}

CommandResult Config::toggleScamFilter()
{
    DbConfigMisc misc = MongoManager::fetchMiscConfig(context->msg.guild_id);
    misc.scamFilterEnabled = !misc.scamFilterEnabled;
    MongoManager::updateMiscConfig(misc);
    return CommandResult::fromSuccess(std::format(Responses::ToggledScamFilter, misc.scamFilterEnabled ? "ON" : "OFF"));
}

CommandResult Config::unwhitelistChannel(const ChannelTypeReader& channelRead)
{
    dpp::channel* channel = channelRead.topResult();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    if (!std::erase(channels.whitelistedChannels, channel->id))
        return CommandResult::fromError(Responses::ChannelNotWhitelisted);

    MongoManager::updateChannelConfig(channels);
    return CommandResult::fromSuccess(std::format(Responses::ChannelUnwhitelisted, channel->get_mention()));
}

CommandResult Config::whitelistChannel(const ChannelTypeReader& channelRead)
{
    dpp::channel* channel = channelRead.topResult();
    DbConfigChannels channels = MongoManager::fetchChannelConfig(context->msg.guild_id);
    channels.whitelistedChannels.push_back(channel->id);
    MongoManager::updateChannelConfig(channels);
    return CommandResult::fromSuccess(std::format(Responses::ChannelWhitelisted, channel->get_mention()));
}
