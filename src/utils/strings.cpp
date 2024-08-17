#include "strings.h"
#include "dpp-command-handler/utils/strings.h"
#include <array>
#include <algorithm>

namespace RR
{
    namespace utility
    {
        bool iStartsWith(std::string_view str, std::string_view substr)
        {
            if (substr.size() > str.size())
                return false;
            return dpp::utility::iequals(str.substr(0, substr.size()), substr);
        }

        std::string& sanitize(std::string& str)
        {
            if (str.empty())
                return str;

            constexpr std::array escapedChars = { "\\*", "\\_", "\\`", "\\~", "\\>" };
            constexpr std::array sensitiveChars = { "*", "_", "`", "~", ">" };

            for (int i = 0; i < sensitiveChars.size(); ++i)
                strReplace(str, sensitiveChars[i], escapedChars[i]);

            return str;
        }

        std::string& strReplace(std::string& str, std::string_view from, std::string_view to)
        {
            size_t pos{};
            while ((pos = str.find(from, pos)) != std::string::npos)
            {
                str.replace(pos, from.size(), to);
                pos += to.size();
            }

            return str;
        }

        std::string& trimZeros(std::string& str)
        {
            if (size_t dotPos = str.find('.'); dotPos != std::string::npos)
            {
                size_t endPos = str.find_last_not_of('0');
                if (endPos == dotPos)
                    str.erase(dotPos);
                else
                    str.erase(endPos + 1);
            }

            return str;
        }

        std::string toLower(std::string_view str)
        {
            std::string out;
            out.reserve(str.size());
            std::ranges::transform(str, std::back_inserter(out), [](unsigned char c) { return std::tolower(c); });
            return out;
        }

        std::string toUpper(std::string_view str)
        {
            std::string out;
            out.reserve(str.size());
            std::ranges::transform(str, std::back_inserter(out), [](unsigned char c) { return std::toupper(c); });
            return out;
        }
    }
}
