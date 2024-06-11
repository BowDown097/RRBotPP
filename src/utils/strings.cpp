#include "strings.h"
#include <algorithm>
#include <array>

namespace RR
{
    namespace utility
    {
        std::string formatPair(const std::pair<std::string, std::string>& pair)
        {
            return "**" + pair.first + "**: " + pair.second;
        }

        std::string sanitizeString(std::string_view str)
        {
            std::string sanitized(str);
            if (str.empty())
                return sanitized;

            constexpr std::array escapedChars = { "\\*", "\\_", "\\`", "\\~", "\\>" };
            constexpr std::array sensitiveChars = { "*", "_", "`", "~", ">" };

            for (int i = 0; i < sensitiveChars.size(); ++i)
                strReplace(sanitized, sensitiveChars[i], escapedChars[i]);

            return sanitized;
        }

        void strReplace(std::string& str, std::string_view from, std::string_view to)
        {
            size_t pos = 0;
            while ((pos = str.find(from, pos)) != std::string::npos)
            {
                str.replace(pos, from.size(), to);
                pos += to.size();
            }
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
