#pragma once
#include "dppcmd/modules/module.h"

class Consumable;
class DbUser;

namespace dpp { class guild_member; }

class Goods : public dppcmd::module<Goods>
{
public:
    Goods();
private:
    dpp::task<dppcmd::command_result> buy(const dppcmd::remainder<std::string>& itemIn);
    dpp::task<dppcmd::command_result> daily();
    dpp::task<dppcmd::command_result> discard(const dppcmd::remainder<std::string>& itemIn);
    dppcmd::command_result itemInfo(const dppcmd::remainder<std::string>& itemIn);
    dppcmd::command_result items(const std::optional<dpp::guild_member>& memberOpt);
    dpp::task<dppcmd::command_result> open(const dppcmd::remainder<std::string>& crateIn);
    dppcmd::command_result shop();

    dpp::task<dppcmd::command_result> use(const dppcmd::remainder<std::string>& consumableIn);

    dpp::task<std::string> genericUse(const std::string& con, DbUser& user, const dpp::guild_member& gm,
                                      std::string_view successMsg, std::string_view loseMsg, int64_t& cooldown,
                                      long double divMin = 2, long double divMax = 5);
};
