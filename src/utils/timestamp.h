#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        std::string formatSeconds(long secs);
        long unixTimestamp(long offset = 0);
    }
}
