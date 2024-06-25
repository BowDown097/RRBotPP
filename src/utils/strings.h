#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        std::string sanitizeString(std::string_view str);
        void strReplace(std::string& str, std::string_view from, std::string_view to);
        std::string toLower(std::string_view str);
        std::string toUpper(std::string_view str);
    }
}
