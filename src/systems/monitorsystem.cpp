#include "monitorsystem.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/dbban.h"
#include "database/entities/dbchill.h"
#include "database/entities/dbpot.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "systems/itemsystem.h"
#include "utils/ld.h"
#include "utils/timestamp.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <dpp/cluster.h>
#include <mongocxx/collection.hpp>

using stream_document = bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace MonitorSystem
{
    static std::unique_ptr<boost::asio::io_context> io_ctx;

    void checkBans(boost::asio::deadline_timer* timer, dpp::cluster* cluster)
    {
        mongocxx::cursor cursor = MongoManager::bans().find(
            stream_document() << "time" << open_document
                << "$lte" << RR::utility::unixTimestamp()
            << close_document << finalize);

        for (bsoncxx::document::view doc : cursor)
        {
            DbBan ban(doc);
            cluster->guild_ban_delete(ban.guildId, ban.userId);
            MongoManager::deleteBan(ban.userId, ban.guildId);
        }

        timer->expires_from_now(boost::posix_time::seconds(30));
        timer->async_wait(std::bind(checkBans, timer, cluster));
    }

    void checkChills(boost::asio::deadline_timer* timer, dpp::cluster* cluster)
    {
        mongocxx::cursor cursor = MongoManager::chills().find(
            stream_document() << "time" << open_document
                << "$lte" << RR::utility::unixTimestamp()
            << close_document << finalize);

        for (bsoncxx::document::view doc : cursor)
        {
            DbChill chill(doc);
            if (dpp::channel* channel = dpp::find_channel(chill.channelId))
            {
                for (dpp::permission_overwrite& overwrite : channel->permission_overwrites)
                {
                    // overwrite with guild ID corresponds to @everyone role
                    if (overwrite.id != chill.guildId || !overwrite.deny.has(dpp::permissions::p_send_messages))
                        continue;

                    overwrite.deny.remove(dpp::permissions::p_send_messages);
                    cluster->channel_edit_permissions(*channel, overwrite.id, overwrite.allow, overwrite.deny, false);
                    cluster->message_create(dpp::message(channel->id, Responses::ChannelThawed));
                    break;
                }
            }
            MongoManager::deleteChill(chill.channelId, chill.guildId);
        }

        timer->expires_from_now(boost::posix_time::seconds(30));
        timer->async_wait(std::bind(checkChills, timer, cluster));
    }

    void checkConsumables(boost::asio::deadline_timer* timer, dpp::cluster* cluster)
    {
        // reset used consumables when their end times have passed up
        constexpr std::array<std::pair<std::string_view, std::string_view>, 4> endTimeMap = {{
            { "Black Hat", "blackHatEndTime" },
            { "Cocaine", "cocaineEndTime" },
            { "Ski Mask", "skiMaskEndTime" },
            { "Viagra", "viagraEndTime" },
        }};

        for (const auto& [name, key] : endTimeMap)
        {
            MongoManager::users().update_many(
                stream_document() << key << open_document
                    << "$gt" << 0 << "$lte" << RR::utility::unixTimestamp()
                << close_document << finalize,
                stream_document() << "$set" << open_document
                    << std::format("usedConsumables.{}", name) << 0
                << close_document << finalize);
        }

        // reset cocaine recovery time when it has passed up
        MongoManager::users().update_many(
            stream_document() << "cocaineRecoveryTime" << open_document
                << "$gt" << 0 << "$lte" << RR::utility::unixTimestamp()
            << close_document << finalize,
            stream_document() << "$set" << open_document
                << "cocaineRecoveryTime" << 0
            << close_document << finalize);

        timer->expires_from_now(boost::posix_time::seconds(30));
        timer->async_wait(std::bind(checkConsumables, timer, cluster));
    }

    void checkPerks(boost::asio::deadline_timer* timer, dpp::cluster* cluster)
    {
        // https://youtu.be/DWtpNPZ4tb4
        mongocxx::cursor cursor = MongoManager::users().find(
            stream_document() << "$expr" << open_document
                << "$gt" << open_array
                    << open_document
                        << "$size" << open_document
                            << "$filter" << open_document
                                << "input" << open_document
                                    << "$objectToArray" << "$perks"
                                << close_document
                                << "cond" << open_document
                                    << "$and" << open_array
                                        << open_document
                                            << "$lte" << open_array
                                                << "$$this.v" << RR::utility::unixTimestamp()
                                            << close_array
                                        << close_document
                                        << open_document
                                            << "$ne" << open_array
                                                << "$$this.k" << "Pacifist"
                                            << close_array
                                        << close_document
                                    << close_array
                                << close_document
                            << close_document
                        << close_document
                    << close_document
                    << 0
                << close_array
            << close_document << finalize);

        for (bsoncxx::document::view doc : cursor)
        {
            DbUser user(doc);
            for (auto it = user.perks.cbegin(); it != user.perks.cend();)
            {
                if (it->second > RR::utility::unixTimestamp() || it->first == "Pacifist")
                {
                    ++it;
                    continue;
                }

                if (it->first == "Multiperk" && user.perks.size() >= 3)
                {
                    std::string lastPerk = user.perks.crbegin()->first;
                    if (const Perk* perk = dynamic_cast<const Perk*>(ItemSystem::getItem(lastPerk)))
                        user.cash += perk->price();
                    user.perks.erase(lastPerk);
                }

                it = user.perks.erase(it);
            }
            MongoManager::updateUser(user);
        }

        timer->expires_from_now(boost::posix_time::seconds(30));
        timer->async_wait(std::bind(checkPerks, timer, cluster));
    }

    void checkPots(boost::asio::deadline_timer* timer, dpp::cluster* cluster)
    {
        mongocxx::cursor cursor = MongoManager::pots().find(
            stream_document() << "endTime" << open_document
                << "$lte" << RR::utility::unixTimestamp()
            << close_document << finalize);

        for (bsoncxx::document::view doc : cursor)
        {
            DbPot pot(doc);
            while (true)
            {
                if (uint64_t luckyGuy = pot.drawMember())
                {
                    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(pot.guildId, luckyGuy);
                    if (!gm)
                    {
                        pot.members.erase(luckyGuy);
                        continue;
                    }

                    DbUser luckyUser = MongoManager::fetchUser(luckyGuy, pot.guildId);
                    long double winnings = pot.value * (1.0L - Constants::PotFee / 100.0L);
                    luckyUser.cash += winnings;

                    DbConfigChannels channels = MongoManager::fetchChannelConfig(pot.guildId);
                    if (channels.potChannel)
                    {
                        cluster->message_create(dpp::message(channels.potChannel, std::format(Responses::PotDrawn,
                            dpp::user::get_mention(luckyGuy), Constants::PotFee,
                            RR::utility::curr2str(winnings), pot.getMemberOdds(luckyGuy))));
                    }

                    MongoManager::updateUser(luckyUser);
                }

                break;
            }

            MongoManager::deletePot(pot.guildId);
        }

        timer->expires_from_now(boost::posix_time::seconds(30));
        timer->async_wait(std::bind(checkPots, timer, cluster));
    }

    void startTimers(dpp::cluster* cluster)
    {
        boost::asio::deadline_timer banTimer(*io_ctx, boost::posix_time::seconds(30));
        banTimer.async_wait(std::bind(checkBans, &banTimer, cluster));

        boost::asio::deadline_timer chillTimer(*io_ctx, boost::posix_time::seconds(30));
        chillTimer.async_wait(std::bind(checkChills, &chillTimer, cluster));

        boost::asio::deadline_timer consumableTimer(*io_ctx, boost::posix_time::seconds(30));
        consumableTimer.async_wait(std::bind(checkConsumables, &consumableTimer, cluster));

        boost::asio::deadline_timer perkTimer(*io_ctx, boost::posix_time::seconds(30));
        perkTimer.async_wait(std::bind(checkPerks, &perkTimer, cluster));

        boost::asio::deadline_timer potTimer(*io_ctx, boost::posix_time::seconds(30));
        potTimer.async_wait(std::bind(checkPots, &potTimer, cluster));

        io_ctx->run();
    }

    void initialize(dpp::cluster* cluster)
    {
        if (io_ctx)
            return;
        io_ctx = std::make_unique<boost::asio::io_context>();
        std::thread timerThread(startTimers, cluster);
        timerThread.detach();
    }
}
