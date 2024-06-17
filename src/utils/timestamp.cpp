#include "timestamp.h"
#include <chrono>
#include <format>

using namespace std::chrono;

namespace RR
{
    namespace utility
    {
        // there is most likely an easier way to do this. that being said, if there is, i couldn't find it.
        std::string formatSeconds(long secs)
        {
            duration<long> duration(secs);
            if (auto h = duration_cast<hours>(duration); h.count() > 0)
            {
                auto m = duration_cast<minutes>(duration - h);
                auto s = duration_cast<seconds>(duration - h - m);
                return std::format("{}:{:02}:{:02}", h.count(), m.count(), s.count());
            }
            else if (auto m = duration_cast<minutes>(duration); m.count() > 0)
            {
                auto s = duration_cast<seconds>(duration - m);
                return std::format("{}:{:02}", m.count(), s.count());
            }

            return std::format("{}s", secs);
        }

        long unixTimestamp(long offset)
        {
            return duration_cast<seconds>(system_clock::now().time_since_epoch()).count() + offset;
        }
    }
}
