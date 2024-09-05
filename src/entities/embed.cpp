#include "embed.h"

namespace RR
{
    embed& embed::add_separator()
    {
        dpp::embed::add_field("\u200b", "\u200b");
        return *this;
    }

    embed& embed::add_string_field(const std::string& name, const std::string& value, bool is_inline,
                                   bool show_if_not_available, const std::string& default_value)
    {
        if (!value.empty() || show_if_not_available)
            dpp::embed::add_field(name, value.empty() ? default_value : value, is_inline);
        return *this;
    }
}
