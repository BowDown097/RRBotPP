#pragma once
#include <condition_variable>
#include <dpp/coro/task.h>
#include <set>

namespace dpp
{
    class cluster;
    class embed;
    class message;
}

class DbConfigMisc;

namespace FilterSystem
{
    bool containsFilteredTerm(std::string_view input, const std::set<std::string>& terms);
    dpp::task<bool> containsInvite(std::string_view input, dpp::cluster* cluster);
    bool containsScam(std::string_view input, const std::vector<dpp::embed>& embeds);
    dpp::task<bool> messageIsBad(const dpp::message& msg, dpp::cluster* cluster, const DbConfigMisc& misc);
}
