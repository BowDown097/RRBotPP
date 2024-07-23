#pragma once
#include <bsoncxx/decimal128-fwd.hpp>
#include <bsoncxx/document/element-fwd.hpp>
#include <optional>
#include <string>

namespace RR
{
    namespace utility
    {
        long double get_long_double(const bsoncxx::document::element& element);
        bsoncxx::decimal128 put_long_double(long double value);

        std::string curr2str(long double value);
        std::optional<long double> str2curr(std::string_view value);

        long double round(long double value, int digits);

    }
}
