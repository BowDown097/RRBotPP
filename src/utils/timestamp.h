#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        std::string formatTimestamp(long timestamp);
        long unixTimestamp(long offset = 0);
    }
}
