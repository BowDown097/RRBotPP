#include "filtersystem.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/mongomanager.h"
#include "utils/strings.h"
#include <boost/url.hpp>
#include <dpp/cluster.h>
#include <regex>

#ifdef RRBOT_HAS_ICU
#include <unicode/uspoof.h>
#define ICU_FAILURE_CHECK(status, message) \
    if (U_FAILURE(status)) \
    { \
        std::cerr << "Filter system failure: " << message << ": " << u_errorName(status) << std::endl; \
        return false; \
    }
#endif

namespace FilterSystem
{
    bool containsFilteredWord(const dpp::snowflake& guildId, std::string_view input)
    {
        DbConfigMisc misc = MongoManager::fetchMiscConfig(guildId);
        std::string inputLower = RR::utility::toLower(input);
        std::string cleaned = inputLower
            | std::views::filter([](unsigned char c) { return !std::isspace(c); })
            | std::ranges::to<std::string>();

    #ifdef RRBOT_HAS_ICU
        UErrorCode status = U_ZERO_ERROR;

        icu::LocalUSpoofCheckerPointer sc(uspoof_open(&status));
        uspoof_setChecks(sc.getAlias(), USPOOF_CONFUSABLE, &status);
        ICU_FAILURE_CHECK(status, "Failed to create spoof checker");

        icu::UnicodeString inputUnicode = icu::UnicodeString::fromUTF8(inputLower);
        icu::UnicodeString inputSkeleton;
        uspoof_getSkeletonUnicodeString(sc.getAlias(), 0, inputUnicode, inputSkeleton, &status);
        ICU_FAILURE_CHECK(status, "Failed to create skeleton for input");

        icu::UnicodeString cleanedUnicode = icu::UnicodeString::fromUTF8(cleaned);
        icu::UnicodeString cleanedSkeleton;
        uspoof_getSkeletonUnicodeString(sc.getAlias(), 0, cleanedUnicode, cleanedSkeleton, &status);
        ICU_FAILURE_CHECK(status, "Failed to create skeleton for cleaned input");

        for (const std::string& term : misc.filteredTerms)
        {
            icu::UnicodeString termUnicode = icu::UnicodeString::fromUTF8(term);
            icu::UnicodeString termSkeleton;
            uspoof_getSkeletonUnicodeString(sc.getAlias(), 0, termUnicode, termSkeleton, &status);
            ICU_FAILURE_CHECK(status, "Failed to create skeleton for filtered term");

            if (std::ranges::any_of(term, [](unsigned char c) { return std::isspace(c); }))
            {
                if (inputSkeleton.indexOf(termSkeleton) != -1)
                    return true;
            }
            else
            {
                if (cleanedSkeleton.indexOf(termSkeleton) != -1)
                    return true;
            }
        }

        return false;
    #else
        return std::ranges::any_of(misc.filteredTerms, [&cleaned, &inputLower](const std::string& term) {
            if (std::ranges::any_of(term, [](unsigned char c) { return std::isspace(c); }))
                return inputLower.contains(term);
            else
                return cleaned.contains(term);
        });
    #endif
    }

    dpp::task<void> doFilteredWordCheck(const dpp::message& message, dpp::cluster* cluster)
    {
        DbConfigChannels channels = MongoManager::fetchChannelConfig(message.guild_id);
        if (std::ranges::contains(channels.noFilterChannels, (int64_t)message.channel_id))
            co_return;
        if (containsFilteredWord(message.guild_id, message.content))
            co_await cluster->co_message_delete(message.id, message.channel_id);
    }

    dpp::task<void> doInviteCheck(const dpp::message& message, dpp::cluster* cluster)
    {
        DbConfigMisc misc = MongoManager::fetchMiscConfig(message.guild_id);
        if (!misc.inviteFilterEnabled)
            co_return;

        DbConfigChannels channels = MongoManager::fetchChannelConfig(message.guild_id);
        if (std::ranges::contains(channels.noFilterChannels, (int64_t)message.channel_id))
            co_return;

        static std::regex inviteRegex(R"(discord(?:\.com\/invite|app\.com\/invite|\.gg|\.me|\.io)\/([a-zA-Z0-9\-]+))");
        std::smatch res;

        for (auto it = message.content.cbegin();
             std::regex_search(it, message.content.cend(), res, inviteRegex);
             it = res.suffix().first)
        {
            dpp::confirmation_callback_t inviteEvent = co_await cluster->co_invite_get(res[1]);
            if (!inviteEvent.is_error())
            {
                co_await cluster->co_message_delete(message.id, message.channel_id);
                break;
            }
        }
    }

    dpp::task<void> doScamCheck(const dpp::message& message, dpp::cluster* cluster)
    {
        DbConfigMisc misc = MongoManager::fetchMiscConfig(message.guild_id);
        if (!misc.scamFilterEnabled)
            co_return;

        DbConfigChannels channels = MongoManager::fetchChannelConfig(message.guild_id);
        if (std::ranges::contains(channels.noFilterChannels, (int64_t)message.channel_id))
            co_return;

        std::string content = RR::utility::toLower(message.content);
        if ((content.contains("skins") && content.contains("imgur")) ||
            (content.contains("nitro") && content.contains("free") && content.contains("http")) ||
            (content.contains("nitro") && content.contains("steam")))
        {
            co_await cluster->co_message_delete(message.id, message.channel_id);
            co_return;
        }

        for (const dpp::embed& embed : message.embeds)
        {
            if (embed.title.empty() || embed.url.empty())
                continue;

            if (boost::system::result<boost::url_view> res = boost::urls::parse_uri(embed.url); !res.has_error())
            {
                std::string host = res->host();
                if (host.empty())
                    continue;

                std::ranges::transform(host, host.begin(), [](unsigned char c) { return std::tolower(c); });
                RR::utility::strReplace(host, "www.", "");

                std::string title = RR::utility::toLower(embed.title);
                if ((title.contains("trade offer") && host != "steamcommunity.com") ||
                    (title.contains("steam community") && host != "steamcommunity.com") ||
                    (title.contains("you've been gifted") && host != "discord.gift") ||
                    (title.contains("nitro") && title.contains("steam")))
                {
                    co_await cluster->co_message_delete(message.id, message.channel_id);
                    break;
                }
            }
        }
    }
}
