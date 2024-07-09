#include "general.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "dpp-command-handler/services/moduleservice.h"
#include "dpp-command-handler/utils/join.h"
#include "utils/dpp.h"
#include <boost/locale/conversion.hpp>
#include <dpp/cluster.h>
#include <dpp/colors.h>

General::General() : dpp::module<General>("General", "The name really explains it all. Fun fact, you used one of the commands under this module to view info about this module.")
{
    register_command(&General::achievements, std::initializer_list<std::string> { "achievements", "ach" }, "View your own or someone else's achievements.", "$achievements <user>");
    register_command(&General::help, "help", "View info about a command.", "$help <command>");
    register_command(&General::info, "info", "View info about the bot.");
    register_command(&General::module, "module", "View info about a module.", "$module [module]");
    register_command(&General::modules, "modules", "View info about the bot's modules.");
    register_command(&General::serverInfo, "serverinfo", "View info about this server.");
    register_command(&General::stats, "stats", "View various statistics about yourself or another user.", "$stats <user>");
    register_command(&General::userInfo, std::initializer_list<std::string> { "userinfo", "whois" }, "View info about yourself or another user.", "$userinfo <user>");
}

dpp::command_result General::achievements(const std::optional<dpp::user_in>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->top_result() : &context->msg.author;
    if (!user)
        return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (dbUser.achievements.empty())
    {
        return dpp::command_result::from_error(user->id == context->msg.author.id
            ? Responses::YouHaveNoAchs : std::format(Responses::UserHasNoAchs, user->get_mention()));
    }

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Achievements")
        .set_description(dpp::utility::join(dbUser.achievements, '\n', [](const auto& p) {
                               return std::format("**{}**: {}", p.first, p.second); }));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result General::help(const std::optional<std::string>& commandName)
{
    if (!commandName)
        return dpp::command_result::from_success(Responses::HelpGenericResponse);

    std::vector<std::reference_wrapper<const dpp::command_info>> commands = service->search_command(commandName.value());
    if (commands.empty())
        return dpp::command_result::from_error(Responses::NonexistentCommand);

    const dpp::command_info& command = commands.front();
    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_description("**" + boost::locale::to_title(command.name()) + "**")
       .add_field("Module", command.module()->name())
       .add_field("Description", command.summary())
       .add_field("Usage", command.remarks())
       .add_field("Aliases", dpp::utility::join(command.aliases(), ", "));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result General::info()
{
    std::span<const std::unique_ptr<dpp::module_base>> modules = service->modules();
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
    return dpp::command_result::from_success();
}

dpp::command_result General::module(const std::string& moduleName)
{
    std::vector<std::reference_wrapper<const dpp::module_base>> modules = service->search_module(moduleName);
    if (modules.empty())
        return dpp::command_result::from_error(Responses::NonexistentModule);

    const dpp::module_base& module = modules.front();
    std::vector<std::reference_wrapper<const dpp::command_info>> commands = module.commands();
    std::ranges::sort(commands, [](const auto& a, const auto& b) { return a.get().name() < b.get().name(); });

    dpp::embed embed = dpp::embed()
       .set_color(dpp::colors::red)
       .set_description("**" + module.name() + "**")
       .add_field("Available commands", dpp::utility::join(commands, ", ", [](const auto& c) { return c.get().name(); }))
       .add_field("Description", module.summary());

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result General::modules()
{
    std::span<const std::unique_ptr<dpp::module_base>> modules = service->modules();
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Modules")
        .set_description(dpp::utility::join(modules, ", ", [](auto& m) { return m->name(); }));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::task<dpp::command_result> General::serverInfo()
{
    dpp::guild* guild = dpp::find_guild(context->msg.guild_id);
    if (!guild)
        co_return dpp::command_result::from_error(Responses::GetGuildFailed);

    dpp::confirmation_callback_t stickersResult = co_await cluster->co_guild_stickers_get(guild->id);
    if (stickersResult.is_error())
        co_return dpp::command_result::from_error(stickersResult.get_error().human_readable);

    std::string banner = guild->get_banner_url();
    std::string discovery = guild->get_discovery_splash_url();
    std::string icon = guild->get_icon_url();
    std::string invSplash = guild->get_splash_url();
    dpp::sticker_map stickers = stickersResult.get<dpp::sticker_map>();

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
    co_return dpp::command_result::from_success();
}

dpp::command_result General::stats(const std::optional<dpp::user_in>& userOpt)
{
    const dpp::user* user = userOpt ? userOpt->top_result() : &context->msg.author;
    if (!user)
        return dpp::command_result::from_error(Responses::GetUserFailed);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser dbUser = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (dbUser.stats.empty())
    {
        return dpp::command_result::from_error(user->id == context->msg.author.id
            ? Responses::YouHaveNoStats : std::format(Responses::UserHasNoStats, user->get_mention()));
    }

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title("Stats")
        .set_description(dpp::utility::join(dbUser.stats | std::ranges::to<std::map>(), '\n', [](const auto& p) {
                               return std::format("**{}**: {}", p.first, p.second); }));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result General::userInfo(const std::optional<dpp::guild_member_in>& memberOpt)
{
    std::optional<dpp::guild_member> member = memberOpt
        ? memberOpt->top_result() : dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        return dpp::command_result::from_error(Responses::GetUserFailed);

    dpp::user* user = member->get_user();
    if (!user)
        return dpp::command_result::from_error(Responses::GetUserFailed);

    dpp::guild* guild = dpp::find_guild(context->msg.guild_id);
    if (!guild)
        return dpp::command_result::from_error(Responses::GetGuildFailed);

    dpp::channel* channel = dpp::find_channel(context->msg.channel_id);
    if (!channel)
        return dpp::command_result::from_error(Responses::GetChannelFailed);

    dpp::permission overwrites = guild->permission_overwrites(member.value(), *channel);
    std::vector<std::pair<dpp::permissions, std::string>> perms = RR::utility::permissionsToList(overwrites);
    auto permNames = perms | std::views::transform([](const auto& p) { return p.second; });

    std::vector<std::string> roleMentions;
    for (dpp::snowflake roleId : member->get_roles())
        if (dpp::role* role = dpp::find_role(roleId))
            roleMentions.push_back(role->get_mention());

    std::string avatarUrl = RR::utility::getDisplayAvatar(member.value(), user);
    dpp::embed embed = dpp::embed()
        .set_author(user->global_name, "", avatarUrl)
        .set_color(dpp::colors::red)
        .set_description("**User Info**")
        .set_thumbnail(avatarUrl)
        .add_field("ID", std::to_string(user->id), true)
        .add_field("Nickname", member->get_nickname(), true)
        .add_field("Joined At", dpp::utility::timestamp(member->joined_at), true)
        .add_field("Created At", dpp::utility::timestamp(user->get_creation_time()), true)
        .add_field("Permissions", dpp::utility::join(permNames, ", "))
        .add_field("Roles", dpp::utility::join(roleMentions, ", "));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}
