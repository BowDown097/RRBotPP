#pragma once
#include "dppcmd/utils/lexical_cast.h"
#include <dpp/message.h>

namespace RR
{
    class embed : public dpp::embed
    {
    public:
        embed& set_title(const std::string& text)
        { dpp::embed::set_title(text); return *this; };
        embed& set_description(const std::string& text)
        { dpp::embed::set_description(text); return *this; }
        embed& set_footer(const dpp::embed_footer& f)
        { dpp::embed::set_footer(f); return *this; }
        embed& set_footer(const std::string& text, const std::string& icon_url = std::string())
        { dpp::embed::set_footer(text, icon_url); return *this; }
        embed& set_color(uint32_t col)
        { dpp::embed::set_color(col); return *this; }
        embed& set_colour(uint32_t col)
        { dpp::embed::set_colour(col); return *this; }
        embed& set_timestamp(time_t tstamp)
        { dpp::embed::set_timestamp(tstamp); return *this; }
        embed& set_url(const std::string& url)
        { dpp::embed::set_url(url); return *this; }
        embed& set_author(const dpp::embed_author& a)
        { dpp::embed::set_author(a); return *this; }
        embed& set_author(const std::string& name, const std::string& url, const std::string& icon_url)
        { dpp::embed::set_author(name, url, icon_url); return *this; }
        embed& set_provider(const std::string& name, const std::string& url)
        { dpp::embed::set_provider(name, url); return *this; }
        embed& set_image(const std::string& url)
        { dpp::embed::set_image(url); return *this; }
        embed& set_video(const std::string& url)
        { dpp::embed::set_video(url); return *this; }
        embed& set_thumbnail(const std::string& url)
        { dpp::embed::set_thumbnail(url); return *this; }

        embed& add_comparison_field(const std::string& name, auto&& value1, auto&& value2,
                                    const std::string& default_value = "N/A")
        {
            if constexpr (std::equality_comparable_with<decltype(value1), decltype(value2)>)
                if (value1 == value2)
                    return *this;

            if constexpr (is_string<decltype(value1)> && is_string<decltype(value2)>)
                add_comparison_fields(name, value1, value2, default_value);
            else if (std::string str1 = lex_to_string(value1), str2 = lex_to_string(value2); str1 != str2)
                add_comparison_fields(name, str1, str2, default_value);

            return *this;
        }

        embed& add_field(const std::string& name, auto&& value, bool condition = true, bool is_inline = false)
        {
            if (condition)
            {
                if constexpr (is_string<decltype(value)>)
                    dpp::embed::add_field(name, value, is_inline);
                else
                    dpp::embed::add_field(name, lex_to_string(value), is_inline);
            }

            return *this;
        }

        embed& add_separator();

        embed& add_string_field(const std::string& name, const std::string& value, bool is_inline = false,
                                bool show_if_not_available = true, const std::string& default_value = "N/A");
    private:
        template<typename T>
        inline static constexpr bool is_string = std::is_same_v<std::decay_t<T>, std::string>;

        inline void add_comparison_fields(const std::string& name, const std::string& str1,
            const std::string& str2, const std::string& default_value)
        {
            dpp::embed::add_field("Previous " + name, !str1.empty() ? str1 : default_value, true);
            dpp::embed::add_field("Current " + name, !str2.empty() ? str2 : default_value, true);
            add_separator();
        }

        inline static std::string lex_to_string(auto&& value)
        {
            return dppcmd::utility::lexical_cast<std::string>(std::forward<decltype(value)>(value), false);
        }
    };
}
