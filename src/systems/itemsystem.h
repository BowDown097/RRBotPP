#pragma once
#include "dpp-command-handler/results/commandresult.h"
#include <dpp/coro/task.h>

class Crate;
class DbUser;
class Item;
class Perk;
class Tool;

namespace dpp { class cluster; class guild_member; class message_create_t; }

namespace ItemSystem
{
    dpp::task<dpp::command_result> buyCrate(const Crate& crate, const dpp::guild_member& member,
                                            DbUser& dbUser, dpp::cluster* cluster, bool notify = true);
    dpp::task<dpp::command_result> buyPerk(const Perk& perk, const dpp::guild_member& member,
                                           DbUser& dbUser, dpp::cluster* cluster);
    dpp::task<dpp::command_result> buyTool(const Tool& tool, const dpp::guild_member& member,
                                           DbUser& dbUser, dpp::cluster* cluster);
    std::string getBestTool(std::span<const std::string> tools, std::string_view type);
    const Item* getItem(std::string_view name);
    void giveCollectible(std::string_view name, const dpp::message_create_t* context, DbUser& dbUser);
}
