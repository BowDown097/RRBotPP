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
            std::ostringstream ss;
            ss.imbue(std::locale(""));
            ss << '$' << std::setprecision(2) << std::fixed << value;
            return ss.str();
        }

        std::optional<long double> strToCurrency(const std::string& value)
        {
            if (size_t ind = value.find('$'); ind != std::string::npos)
            {
                std::istringstream ss(value.substr(ind + 1));
                ss.imbue(std::locale(""));

                long double result;
                if (!(ss >> std::fixed >> result).fail())
                    return result;
            }

            return std::nullopt;
        }
    }
}
