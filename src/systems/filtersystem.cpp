#include "filtersystem.h"
#include "database/entities/config/dbconfigchannels.h"
#include "database/entities/config/dbconfigmisc.h"
#include "database/mongomanager.h"
#include "utils/ranges.h"
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
        auto cleanedIt = inputLower | std::views::filter([](unsigned char c) { return !std::isspace(c); });
        std::string cleaned(cleanedIt.begin(), cleanedIt.end());

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
                return inputLower.find(term) != std::string::npos;
            else
                return cleaned.find(term) != std::string::npos;
        });
    #endif
    }

    dpp::task<void> doFilteredWordCheck(const dpp::message& message, dpp::cluster* cluster)
    {
        DbConfigChannels channels = MongoManager::fetchChannelConfig(message.guild_id);
        if (RR::utility::rangeContains(channels.noFilterChannels, (int64_t)message.channel_id))
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
        if (RR::utility::rangeContains(channels.noFilterChannels, (int64_t)message.channel_id))
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
        if (RR::utility::rangeContains(channels.noFilterChannels, (int64_t)message.channel_id))
            co_return;

        std::string content = RR::utility::toLower(message.content);
        if ((content.find("skins") != std::string::npos && content.find("imgur") != std::string::npos) ||
            (content.find("nitro") != std::string::npos && content.find("free") != std::string::npos && content.find("http") != std::string::npos) ||
            (content.find("nitro") != std::string::npos && content.find("steam") != std::string::npos))
        {
            co_await cluster->co_message_delete(message.id, message.channel_id);
            co_return;
        }

        for (const dpp::embed& embed : message.embeds)
        {
            if (embed.title.empty())
                continue;

            if (boost::system::result<boost::url_view> res = boost::urls::parse_uri(embed.url); !res.has_error())
            {
                std::string host = res->host();
                if (host.empty())
                    continue;

                RR::utility::strReplace(host, "www.", "");
                std::ranges::transform(host, host.begin(), [](unsigned char c) { return std::tolower(c); });

                std::string title = RR::utility::toLower(embed.title);
                if ((title.find("trade offer") != std::string::npos && host != "steamcommunity.com") ||
                    (title.find("steam community") != std::string::npos && host != "steamcommunity.com") ||
                    (title.find("you've been gifted") != std::string::npos && host != "discord.gift") ||
                    (title.find("nitro") != std::string::npos && title.find("steam") != std::string::npos))
                {
                    co_await cluster->co_message_delete(message.id, message.channel_id);
                    break;
                }
            }
        }
    }
}
