#include "general.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/moduleservice.h"
#include "dpp-command-handler/utils/join.h"
#include "utils/rrutils.h"
#include <boost/locale/conversion.hpp>
#include <dpp/cluster.h>
#include <dpp/colors.h>

General::General() : ModuleBase("General", "The name really explains it all. Fun fact, you used one of the commands under this module to view info about this module.")
{
    registerCommand(&General::achievements, std::initializer_list<std::string> { "achievements", "ach" }, "View your own or someone else's achievements.", "$achievements <user>");
    registerCommand(&General::help, "help", "View info about a command.", "$help <command>");
    registerCommand(&General::info, "info", "View info about the bot.");
    registerCommand(&General::module, "module", "View info about a module.", "$module [module]");
    registerCommand(&General::modules, "modules", "View info about the bot's modules.");
    registerCommand(&General::serverInfo, "serverinfo", "View info about this server.");
    registerCommand(&General::stats, "stats", "View various statistics about yourself or another user.", "$stats <user>");
    registerCommand(&General::userInfo, std::initializer_list<std::string> { "userinfo", "whois" }, "View info about yourself or another user.", "$userinfo <user>");
}

CommandResult General::achievements(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (dbUser.achievements.empty())
    {
        return CommandResult::fromError(user->id == context->msg.author.id
            ? Responses::YouHaveNoAchs : std::format(Responses::UserHasNoAchs, user->get_mention()));
    }

    std::string description = std::accumulate(std::next(dbUser.achievements.cbegin()), dbUser.achievements.cend(),
                                              RR::utility::formatPair(*dbUser.achievements.cbegin()),
                                              [](std::string a, auto& b) { return a + '\n' + RR::utility::formatPair(b); });

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Achievements")
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult General::help(const std::optional<std::string>& commandName)
{
    if (!commandName)
        return CommandResult::fromSuccess(Responses::HelpGenericResponse);

    std::vector<std::reference_wrapper<const CommandInfo>> commands = service->searchCommand(commandName.value());
    if (commands.empty())
        return CommandResult::fromError(Responses::NonexistentCommand);

    const CommandInfo& command = commands.front();
    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_description("**" + boost::locale::to_title(command.name()) + "**")
       .add_field("Module", command.module()->name())
       .add_field("Description", command.summary())
       .add_field("Usage", command.remarks())
       .add_field("Aliases", dpp::utility::join(command.aliases(), ", "));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult General::info()
{
    std::span<const std::unique_ptr<ModuleBase>> modules = service->modules();
    uint32_t commandCount = std::accumulate(modules.begin(), modules.end(), 0,
                                            [](uint32_t a, auto& b) { return a + b->commands().size(); });

    dpp::embed embed = dpp::embed()
        .set_author(Responses::InfoTitle, "", cluster->me.get_avatar_url())
        .set_color(dpp::colors::red)
        .set_description(Responses::InfoDescription)
        .add_field("Serving", std::format("{} users across {} servers", dpp::get_user_count(), dpp::get_guild_count()), true)
        .add_field("Latency", std::format("{}ms", round(cluster->rest_ping * 100)), true)
        .add_field("Commands", std::to_string(commandCount), true)
        .add_field("Modules", std::to_string(service->modules().size()), true)
        .add_field("Support Discord", Responses::InfoSupportDiscord, true)
        .set_footer(Responses::InfoFooter, "");

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult General::module(const std::string& moduleName)
{
    std::vector<std::reference_wrapper<const ModuleBase>> modules = service->searchModule(moduleName);
    if (modules.empty())
        return CommandResult::fromError(Responses::NonexistentModule);

    const ModuleBase& module = modules.front();
    std::vector<std::reference_wrapper<const CommandInfo>> commands = module.commands();
    std::ranges::sort(commands, [](auto a, auto b) { return a.get().name() < b.get().name(); });
    std::string commandsList = std::accumulate(std::next(commands.cbegin()), commands.cend(), commands.front().get().name(),
                                               [](std::string a, auto b) { return a + ", " + b.get().name(); });

    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_description("**" + module.name() + "**")
       .add_field("Available commands", commandsList)
       .add_field("Description", module.summary());

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult General::modules()
{
    std::span<const std::unique_ptr<ModuleBase>, std::dynamic_extent> modules = service->modules();
    std::string modulesList = std::accumulate(std::next(modules.begin()), modules.end(), modules.front()->name(),
                                              [](std::string a, auto& b) { return a + ", " + b->name(); });

    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_title("Modules")
       .set_description(modulesList);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

dpp::task<CommandResult> General::serverInfo()
{
    dpp::guild* guild = dpp::find_guild(context->msg.guild_id);
    if (!guild)
        co_return CommandResult::fromError(Responses::GetGuildFailed);

    auto stickers = co_await RR::utility::co_get<dpp::sticker_map>(std::move(cluster->co_guild_stickers_get(guild->id)));

    std::string banner = guild->get_banner_url();
    std::string discovery = guild->get_discovery_splash_url();
    std::string icon = guild->get_icon_url();
    std::string invSplash = guild->get_splash_url();

    uint32_t categories{}, textChannels{}, threads = guild->threads.size(), voiceChannels{};
    for (const dpp::snowflake& snowflake : guild->channels)
    {
        if (dpp::channel* channel = dpp::find_channel(snowflake))
        {
            if (channel->is_category())
                categories++;
            else if (channel->is_text_channel())
                textChannels++;
            else if (channel->is_voice_channel())
                voiceChannels++;
        }
    }

    dpp::embed embed = dpp::embed()
        .set_author(guild->name, "", icon)
        .set_color(dpp::colors::red)
        .set_description("**Server Info**")
        .set_thumbnail(icon)
        .add_field("Banner", !banner.empty() ? "[Here](" + banner + ")" : "N/A", true)
        .add_field("Discovery Splash", !discovery.empty() ? "[Here](" + banner + ")" : "N/A", true)
        .add_field("Icon", !icon.empty() ? "[Here](" + icon + ")" : "N/A", true)
        .add_field("Invite Splash", !invSplash.empty() ? "[Here](" + invSplash + ")" : "N/A", true)
        .add_field("Categories", std::to_string(categories), true)
        .add_field("Text Channels", std::to_string(textChannels), true)
        .add_field("Threads", std::to_string(threads), true)
        .add_field("Voice Channels", std::to_string(voiceChannels), true)
        .add_field("Boosts", std::to_string(guild->premium_subscription_count), true)
        .add_field("Emotes", std::to_string(guild->emojis.size()), true)
        .add_field("Members", std::to_string(guild->members.size()), true)
        .add_field("Roles", std::to_string(guild->roles.size()), true)
        .add_field("Stickers", std::to_string(stickers.size()), true)
        .add_field("Created At", dpp::utility::timestamp(guild->get_creation_time()), true)
        .add_field("Description", !guild->description.empty() ? guild->description : "N/A", true)
        .add_field("ID", std::to_string(guild->id), true)
        .add_field("Owner", dpp::user::get_mention(guild->owner_id), true)
        .add_field("Vanity URL", !guild->vanity_url_code.empty() ? guild->vanity_url_code : "N/A", true);

    context->reply(dpp::message(context->msg.channel_id, embed));
    co_return CommandResult::fromSuccess();
}

CommandResult General::stats(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);
    if (user->is_bot())
        return CommandResult::fromError(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (dbUser.stats.empty())
    {
        return CommandResult::fromError(user->id == context->msg.author.id
            ? Responses::YouHaveNoStats : std::format(Responses::UserHasNoStats, user->get_mention()));
    }

    std::map<std::string, std::string> stats(std::make_move_iterator(dbUser.stats.begin()),
                                             std::make_move_iterator(dbUser.stats.end()));
    std::string description = std::accumulate(std::next(stats.cbegin()), stats.cend(),
                                              RR::utility::formatPair(*stats.cbegin()),
                                              [](std::string a, auto& b) { return a + '\n' + RR::utility::formatPair(b); });

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Stats")
        .set_description(description);

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}

CommandResult General::userInfo(const std::optional<UserTypeReader>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->topResult() : &context->msg.author;
    if (!user)
        return CommandResult::fromError(Responses::GetUserFailed);

    dpp::guild* guild = dpp::find_guild(context->msg.guild_id);
    if (!guild)
        return CommandResult::fromError(Responses::GetGuildFailed);

    std::optional<dpp::guild_member> guildMember = RR::utility::findGuildMember(guild->id, user->id);
    if (!guildMember)
        return CommandResult::fromError(Responses::GetUserFailed);

    dpp::channel* channel = dpp::find_channel(context->msg.channel_id);
    if (!channel)
        return CommandResult::fromError(Responses::GetChannelFailed);

    dpp::permission overwrites = guild->permission_overwrites(guildMember.value(), *channel);
    std::vector<std::pair<dpp::permissions, std::string>> perms = RR::utility::permissionsToList(overwrites);
    auto permNames = perms | std::views::transform([](const auto& p) { return p.second; });

    std::vector<std::string> roleMentions;
    for (dpp::snowflake roleId : guildMember->get_roles())
        if (dpp::role* role = dpp::find_role(roleId))
            roleMentions.push_back(role->get_mention());

    std::string avatarUrl = RR::utility::getDisplayAvatar(guildMember.value(), user);
    dpp::embed embed = dpp::embed()
        .set_author(user->global_name, "", avatarUrl)
        .set_color(dpp::colors::red)
        .set_description("**User Info**")
        .set_thumbnail(avatarUrl)
        .add_field("ID", std::to_string(user->id), true)
        .add_field("Nickname", guildMember->get_nickname(), true)
        .add_field("Joined At", dpp::utility::timestamp(guildMember->joined_at), true)
        .add_field("Created At", dpp::utility::timestamp(user->get_creation_time()), true)
        .add_field("Permissions", dpp::utility::join(permNames, ", "))
        .add_field("Roles", dpp::utility::join(roleMentions, ", "));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return CommandResult::fromSuccess();
}
