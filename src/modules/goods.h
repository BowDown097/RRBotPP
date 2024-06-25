#pragma once
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Consumable;
class DbUser;

namespace dpp { class guild_member; }

class Goods : public dpp::module<Goods>
{
public:
    Goods();
private:
    dpp::task<dpp::command_result> buy(const dpp::remainder<std::string>& itemIn);
    dpp::task<dpp::command_result> daily();
    dpp::task<dpp::command_result> discard(const dpp::remainder<std::string>& itemIn);
    dpp::command_result itemInfo(const dpp::remainder<std::string>& itemIn);
    dpp::task<dpp::command_result> items(const std::optional<dpp::user_in>& userIn);
    dpp::task<dpp::command_result> open(const dpp::remainder<std::string>& crateIn);
    dpp::task<dpp::command_result> shop();

    dpp::task<dpp::command_result> use(const dpp::remainder<std::string>& consumableIn);

    dpp::task<std::string> genericUse(const std::string& con, DbUser& user, const dpp::guild_member& gm,
                                      std::string_view successMsg, std::string_view loseMsg, int64_t& cooldown,
                                      long double divMin = 2, long double divMax = 5);
};
