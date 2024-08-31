#include "events.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigglobal.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/commands/commandinfo.h"
#include "dppcmd/commands/exceptions.h"
#include "dppcmd/extensions/cache.h"
#include "dppcmd/modules/modulebase.h"
#include "dppcmd/services/moduleservice.h"
#include "systems/filtersystem.h"
#include "systems/itemsystem.h"
#include "utils/dpp.h"
#include "utils/random.h"
#include "utils/timestamp.h"
#include <dpp/cluster.h>

dpp::task<void> onGuildCreate(const dpp::guild_create_t& event, dpp::cluster* cluster)
{
    if (!event.created)
        co_return;

    // this event is fired for any guild that becomes available to the bot, not when a guild is first joined.
    // so, we check the join date of the bot user, with a 10 second margin of error to be safe.
    if (co_await RR::utility::getJoinTime(cluster, event.created, cluster->me.id) < RR::utility::unixTimestamp(-10))
        co_return;

    if (dpp::channel* defaultChannel = RR::utility::getDefaultChannel(event.created->channels, &cluster->me))
        co_await cluster->co_message_create(dpp::message(defaultChannel->id, Responses::JoinMessage));
}

dpp::task<void> onMessageCreate(const dpp::message_create_t& event, dpp::cluster* cluster, dppcmd::module_service* modules)
{
    const std::string& content = event.msg.content;
    if (content.empty() || event.msg.author.is_bot())
        co_return;

    co_await FilterSystem::doFilteredWordCheck(event.msg, cluster);
    co_await FilterSystem::doInviteCheck(event.msg, cluster);
    co_await FilterSystem::doScamCheck(event.msg, cluster);

    DbUser user = MongoManager::fetchUser(event.msg.author.id, event.msg.guild_id);
    if (content.starts_with(modules->config().command_prefix))
    {
        try
        {
            std::vector<const dppcmd::command_info*> cmds = modules->search_command(
                content.substr(1, content.find(modules->config().separator_char) - 1));
            if (cmds.empty())
                co_return;

            if (user.usingSlots)
            {
                event.reply(Responses::YouAreGambling);
                co_return;
            }

            DbConfigChannels channels = MongoManager::fetchChannelConfig(event.msg.guild_id);
            const dppcmd::command_info* cmd = cmds.front();
            constexpr std::array exemptModules = { "Administration", "BotOwner", "Config", "Moderation" };

            if (!std::ranges::contains(exemptModules, cmd->module()->name()) &&
                !channels.whitelistedChannels.empty() &&
                !channels.whitelistedChannels.contains(event.msg.channel_id))
            {
                event.reply(Responses::CommandsDisabled);
                co_return;
            }

            DbConfigGlobal globalConfig = MongoManager::fetchGlobalConfig();
            if (globalConfig.bannedUsers.contains(event.msg.author.id))
            {
                event.reply(Responses::BannedFromBot);
                co_return;
            }

            DbConfigMisc misc = MongoManager::fetchMiscConfig(event.msg.guild_id);
            if (globalConfig.disabledCommands.contains(cmd->name()) || misc.disabledCommands.contains(cmd->name()))
            {
                event.reply(Responses::CommandDisabled);
                co_return;
            }
            if (misc.disabledModules.contains(cmd->module()->name()))
            {
                event.reply(Responses::ModuleDisabled);
                co_return;
            }

            dppcmd::command_result result = co_await modules->handle_message(&event);
            if (result.message().empty())
                co_return;

            std::optional<dppcmd::command_error> error = result.error();
            using CmdErr = dppcmd::command_error;
            if (result.success() || error == CmdErr::unsuccessful || error == CmdErr::unmet_precondition)
                event.reply(result.message());
        }
        catch (const dppcmd::bad_argument_count& ex)
        {
            std::vector<const dppcmd::command_info*> cmds = modules->search_command(ex.command());
            event.reply(!cmds.empty()
                ? std::format(Responses::BadArgCount, ex.target_arg_count(), cmds.front()->remarks())
                : std::format(Responses::ErrorOccurred, ex.what()));
        }
        catch (const dppcmd::bad_command_argument& ex)
        {
            if (ex.error() == dppcmd::command_error::multiple_matches)
            {
                event.reply(ex.message());
                co_return;
            }

            std::vector<const dppcmd::command_info*> cmds = modules->search_command(ex.command());
            event.reply(!cmds.empty()
                ? std::format(Responses::BadArgument, ex.arg(), ex.message(), cmds.front()->remarks())
                : std::format(Responses::ErrorOccurred, ex.what()));
        }
        catch (const std::exception& ex)
        {
            event.reply(std::format(Responses::ErrorOccurred, ex.what()));
        }
    }
    else
    {
        if (user.messageCashCooldown <= 0)
        {
            user.messageCashCooldown = RR::utility::unixTimestamp(Constants::MessageCashCooldown);
        }
        else if (user.messageCashCooldown <= RR::utility::unixTimestamp())
        {
            auto member = dppcmd::find_guild_member_opt(event.msg.guild_id, event.msg.author.id);
            if (!member)
                co_return;

            co_await user.setCash(member.value(), user.cash + Constants::MessageCash, cluster);
            user.modCooldown(user.messageCashCooldown = Constants::MessageCashCooldown, member.value());

            DbConfigMisc misc = MongoManager::fetchMiscConfig(event.msg.guild_id);
            if (!misc.dropsDisabled && RR::utility::random(70) == 1)
                ItemSystem::giveCollectible("Bank Cheque", &event, user);

            if (user.cash >= 1000000.0L && !user.hasReachedAMilli)
            {
                user.hasReachedAMilli = true;
                ItemSystem::giveCollectible("V Card", &event, user);
            }
        }

        MongoManager::updateUser(user);
    }
}

void Events::connectEvents(dpp::cluster* cluster, dppcmd::module_service* modules)
{
    cluster->on_guild_create(std::bind(&onGuildCreate, std::placeholders::_1, cluster));
    cluster->on_log(dpp::utility::cout_logger());
    cluster->on_message_create(std::bind(&onMessageCreate, std::placeholders::_1, cluster, modules));
}
