#pragma once
#include <ranges>

namespace RR
{
    namespace utility
    {
        // https://en.cppreference.com/w/cpp/algorithm/ranges/contains
        // clang doesn't have this in any actual release yet (will come in 19)
        template<std::ranges::input_range R, class T, class Proj = std::identity>
        requires std::indirect_binary_predicate<std::ranges::equal_to,
                                                std::projected<std::ranges::iterator_t<R>, Proj>,
                                                const T*>
        bool rangeContains(R&& range, const T& value, Proj proj = {})
        {
            return std::ranges::find(range, value, proj) != std::ranges::end(range);
        }
    }
}
