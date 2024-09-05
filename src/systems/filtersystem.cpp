#include "filtersystem.h"
#include "database/entities/config/dbconfigmisc.h"
#include "utils/regex.h"
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
    bool containsFilteredTerm(std::string_view input, const std::set<std::string>& terms)
    {
        if (terms.empty())
            return false;

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

        for (const std::string& term : terms)
        {
            icu::UnicodeString termUnicode = icu::UnicodeString::fromUTF8(term);
            icu::UnicodeString termSkeleton;
            uspoof_getSkeletonUnicodeString(sc.getAlias(), 0, termUnicode, termSkeleton, &status);
            ICU_FAILURE_CHECK(status, "Failed to create skeleton for filtered term");

            bool hasWhitespace = std::ranges::any_of(term, [](unsigned char c) { return std::isspace(c); });
            if ((hasWhitespace ? inputSkeleton : cleanedSkeleton).indexOf(termSkeleton) != -1)
                return true;
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

    dpp::task<bool> containsInvite(std::string_view input, dpp::cluster* cluster)
    {
        static std::regex inviteRegex(R"(discord(?:\.com\/invite|app\.com\/invite|\.gg|\.me|\.io)\/([a-zA-Z0-9\-]+))");
        RR::utility::svmatch res;

        for (auto it = input.cbegin(); std::regex_search(it, input.cend(), res, inviteRegex); it = res.suffix().first)
        {
            dpp::confirmation_callback_t inviteEvent = co_await cluster->co_invite_get(res[1].str());
            if (!inviteEvent.is_error())
                co_return true;
        }

        co_return false;
    }

    bool containsScam(std::string_view input, const std::vector<dpp::embed>& embeds)
    {
        std::string content = RR::utility::toLower(input);
        if ((content.contains("skins") && content.contains("imgur")) ||
            (content.contains("nitro") && content.contains("free") && content.contains("http")) ||
            (content.contains("nitro") && content.contains("steam")))
        {
            return true;
        }

        for (const dpp::embed& embed : embeds)
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
                    return true;
                }
            }
        }

        return false;
    }

    dpp::task<bool> messageIsBad(const dpp::message& msg, dpp::cluster* cluster, const DbConfigMisc& misc)
    {
        co_return FilterSystem::containsFilteredTerm(msg.content, misc.filteredTerms)
            || (misc.inviteFilterEnabled && co_await FilterSystem::containsInvite(msg.content, cluster))
            || (misc.scamFilterEnabled && FilterSystem::containsScam(msg.content, msg.embeds));
    }
}
