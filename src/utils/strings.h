#pragma once
#include <string>

namespace RR
{
    namespace utility
    {
        bool iStartsWith(std::string_view str, std::string_view substr);
        std::string& sanitize(std::string& str);
        std::string& strReplace(std::string& str, std::string_view from, std::string_view to);
        std::string& trimZeros(std::string& str);
        std::string toLower(std::string_view str);
        std::string toUpper(std::string_view str);
    }
}
