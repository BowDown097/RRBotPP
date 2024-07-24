#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        std::string formatSeconds(long secs);
        long resolveDurationString(std::string_view durationStr);
        long secondsInMinutes(unsigned count);
        long secondsInHours(unsigned count);
        long secondsInDays(unsigned count);
        long unixTimestamp(long offset = 0);
    }
}
