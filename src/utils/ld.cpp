#include "ld.h"
#include "dppcmd/utils/lexical_cast.h"
#include "utils/strings.h"
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>
#include <cmath>

namespace RR
{
    namespace utility
    {
        // lexical_cast is used here instead of creating from high and low bits
        // because long double is usually 80 bits sadly, not 128
        long double get_long_double(const bsoncxx::document::element& element)
        {
            return element
                ? dppcmd::utility::lexical_cast<long double>(element.get_decimal128().value.to_string())
                : (long double){};
        }

        bsoncxx::decimal128 put_long_double(long double value)
        {
            return bsoncxx::decimal128(dppcmd::utility::lexical_cast<std::string>(value));
        }

        std::string cash2str(long double value)
        {
            std::ostringstream ss;
            ss.imbue(std::locale(""));
            ss << '$' << std::setprecision(2) << std::fixed << value;
            return ss.str();
        }

        std::optional<long double> str2cash(std::string_view value)
        {
            if (size_t ind = value.find('$'); ind != std::string_view::npos)
            {
                std::string_view cashValue = value.substr(ind + 1);
                // i hope the string values for these numbers aren't implementation defined...
                if (cashValue == "nan") [[unlikely]]
                    return std::numeric_limits<long double>::quiet_NaN();
                if (cashValue == "inf") [[unlikely]]
                    return std::numeric_limits<long double>::infinity();
                if (cashValue == "-inf") [[unlikely]]
                    return -std::numeric_limits<long double>::infinity();

                dppcmd::utility::memstreambuf sbuf(cashValue.data(), cashValue.size());
                std::istream in(&sbuf);
                in.imbue(std::locale(""));

                long double result;
                if (!(in >> std::fixed >> result).fail())
                    return result;
            }

            return std::nullopt;
        }

        long double round(long double value, int digits)
        {
            constexpr int digits10 = std::numeric_limits<long double>::digits10;
            if (digits > digits10)
                throw std::out_of_range(std::format("Rounding beyond max of {} digits for long double", digits10));

            if (std::abs(value) < std::pow(10, digits10 + 1))
            {
                long double scaleFactor = std::pow(10, digits);
                value = std::round(value * scaleFactor) / scaleFactor;
            }

            return value;
        }

        std::string roundAsStr(long double value, int digits)
        {
            std::string formatString = std::format("{{:.{}f}}", digits);
            std::string result = std::vformat(formatString, std::make_format_args(value));
            return trimZeros(result);
        }
    }
}
