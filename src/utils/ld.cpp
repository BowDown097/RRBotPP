#include "ld.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>

namespace RR
{
    namespace utility
    {
        // lexical_cast is used here instead of creating from high and low bits
        // because long double is usually 80 bits sadly, not 128
        long double get_long_double(const bsoncxx::document::element& element)
        {
            return dpp::utility::lexical_cast<long double>(element.get_decimal128().value.to_string());
        }

        bsoncxx::decimal128 put_long_double(long double value)
        {
            return bsoncxx::decimal128(dpp::utility::lexical_cast<std::string>(value));
        }

        std::string currencyToStr(long double value)
        {
            return std::format("${:.2f}", value);
        }

        std::optional<long double> strToCurrency(const std::string& value)
        {
            if (size_t ind = value.find('$'); ind != std::string::npos)
            {
                try
                {
                    return dpp::utility::lexical_cast<long double>(value.substr(ind + 1));
                }
                catch (const dpp::utility::bad_lexical_cast&) {}
            }

            return std::nullopt;
        }
    }
}
