#include "strings.h"
#include <array>

namespace RR
{
    namespace utility
    {
        std::string formatPair(const std::pair<std::string, std::string>& pair)
        {
            return "**" + pair.first + "**: " + pair.second;
        }

        std::string sanitizeString(const std::string& str)
        {
            if (str.empty())
                return str;

            constexpr std::array escapedChars = { "\\*", "\\_", "\\`", "\\~", "\\>" };
            constexpr std::array sensitiveChars = { "*", "_", "`", "~", ">" };

            std::string sanitized(str);
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
    }
}
