#pragma once
#include <regex>

namespace RR
{
    namespace utility
    {
        using svmatch = std::match_results<std::string_view::iterator>;

        // https://stackoverflow.com/a/37516316
        template<class BiDirIt, class CharT, class Traits, class UnaryFunction>
        std::basic_string<CharT> regex_replace(BiDirIt first, BiDirIt last,
            const std::basic_regex<CharT, Traits>& re, UnaryFunction&& f)
        {
            std::basic_string<CharT> s;
            typename std::match_results<BiDirIt>::difference_type lastMatchPos = 0;
            auto lastMatchEnd = first;

            auto callback = [&](const std::match_results<BiDirIt>& match) {
                auto thisMatchPos = match.position();
                auto diff = thisMatchPos - lastMatchPos;

                auto thisMatchStart = lastMatchEnd;
                std::advance(thisMatchStart, diff);

                s.append(lastMatchEnd, thisMatchStart);
                s.append(f(match));

                auto matchLength = match.length();
                lastMatchPos = thisMatchPos + matchLength;
                lastMatchEnd = thisMatchStart;
                std::advance(lastMatchEnd, matchLength);
            };

            std::regex_iterator<BiDirIt> begin(first, last, re), end;
            std::for_each(begin, end, callback);
            s.append(lastMatchEnd, last);
            return s;
        }
    }
}
