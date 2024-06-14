#pragma once
#include <limits>
#include <ranges>
#include <sodium.h>

namespace RR
{
    namespace utility
    {
        template<std::integral T>
        T random(T min, T max)
        {
            return min + randombytes_uniform(max - min);
        }

        template<std::floating_point T>
        T random(T min, T max)
        {
            constexpr int shift = std::min(std::numeric_limits<T>::digits, std::numeric_limits<double>::digits);
            uint64_t value;
            randombytes_buf(&value, sizeof(value));
            T flt = (T)(value >> 11) / (T)(1ull << shift);
            return min + flt * (max - min);
        }

        template<typename T> requires std::integral<T> || std::floating_point<T>
        T random(T max) { return random((T)0, max); }

        template<typename T>
        using ref_if_not_trivial = std::conditional_t<std::is_trivially_copyable_v<T>, T, T&>;

        template<std::ranges::range Range>
        ref_if_not_trivial<std::ranges::range_value_t<Range>> randomElement(Range&& range)
        {
            if constexpr (std::ranges::input_range<Range>)
                return range[random(std::ranges::size(range))];
            else if constexpr (std::ranges::sized_range<Range>)
                return *std::ranges::next(std::ranges::begin(range), random(std::ranges::size(range)));
            else
                return *std::ranges::next(std::ranges::begin(range), random(std::ranges::distance(range)));
        }
    }
}
