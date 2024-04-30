#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        std::string formatPair(const std::pair<std::string, std::string>& pair);
        std::string sanitizeString(const std::string& str);
        void strReplace(std::string& str, std::string_view from, std::string_view to);
    }
}
