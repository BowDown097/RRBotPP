#include "timestamp.h"
#include "dppcmd/utils/lexical_cast.h"
#include <chrono>
#include <format>

namespace RR
{
    namespace utility
    {
        // there is most likely an easier way to do this. that being said, if there is, i couldn't find it.
        std::string formatSeconds(long secs)
        {
            if (secs < 0)
                return "Indefinite";

            std::chrono::duration<long> duration(secs);
            if (auto h = std::chrono::duration_cast<std::chrono::hours>(duration); h.count() > 0)
            {
                auto m = std::chrono::duration_cast<std::chrono::minutes>(duration - h);
                auto s = std::chrono::duration_cast<std::chrono::seconds>(duration - h - m);
                return std::format("{}:{:02}:{:02}", h.count(), m.count(), s.count());
            }
            else if (auto m = std::chrono::duration_cast<std::chrono::minutes>(duration); m.count() > 0)
            {
                auto s = std::chrono::duration_cast<std::chrono::seconds>(duration - m);
                return std::format("{}:{:02}", m.count(), s.count());
            }

            return std::format("{}s", secs);
        }

        long resolveDurationString(std::string_view durationStr)
        {
            if (durationStr.empty())
                return 0;

            try
            {
                long secs{};
                long duration = dppcmd::utility::lexical_cast<long>(durationStr.substr(0, durationStr.size() - 1));
                switch (durationStr.back())
                {
                case 's':
                    secs = duration;
                    break;
                case 'm':
                    secs = secondsInMinutes(duration);
                    break;
                case 'h':
                    secs = secondsInHours(duration);
                    break;
                case 'd':
                    secs = secondsInDays(duration);
                    break;
                }

                return secs;
            }
            catch (const dppcmd::utility::bad_lexical_cast& e) {}

            return 0;
        }

        long secondsInMinutes(unsigned count)
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::minutes(count)).count();
        }

        long secondsInHours(unsigned count)
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(count)).count();
        }

        long secondsInDays(unsigned count)
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::days(count)).count();
        }

        long unixTimestamp(long offset)
        {
            std::chrono::duration sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
            return std::chrono::duration_cast<std::chrono::seconds>(sinceEpoch).count() + offset;
        }
    }
}
