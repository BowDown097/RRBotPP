#include "tasks.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/extensions/cache.h"
#include "systems/itemsystem.h"
#include "utils/ld.h"
#include "utils/random.h"
#include <dpp/dispatcher.h>

Tasks::Tasks() : dppcmd::module<Tasks>("Tasks", "The best way to earn money, at least for those lucky or rich enough to get themselves a tool.")
{
    register_command(&Tasks::chop, "chop", "Go chop some wood.");
    register_command(&Tasks::dig, "dig", "Go digging.");
    register_command(&Tasks::farm, "farm", "Go farming.");
    register_command(&Tasks::fish, "fish", "Go fishing.");
    register_command(&Tasks::hunt, "hunt", "Go hunting.");
    register_command(&Tasks::mine, "mine", "Go mining.");
}

dpp::task<dppcmd::command_result> Tasks::chop()
{ co_return co_await genericTask("Axe", "chopped down", "trees", "Chop", Constants::ChopCooldown); }
dpp::task<dppcmd::command_result> Tasks::dig()
{ co_return co_await genericTask("Shovel", "mined", "dirt", "Dig", Constants::DigCooldown); }
dpp::task<dppcmd::command_result> Tasks::farm()
{ co_return co_await genericTask("Hoe", "farmed", "crops", "Farm", Constants::FarmCooldown); }
dpp::task<dppcmd::command_result> Tasks::hunt()
{ co_return co_await genericTask("Sword", "hunted", "mobs", "Hunt", Constants::HuntCooldown); }

dpp::task<dppcmd::command_result> Tasks::fish()
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    const std::pair<std::string_view, long double>& fishPair = RR::utility::randomElement(Constants::Fish);
    int numCaught = RR::utility::random(7, 15);

    if (user.perks.contains("Enchanter"))
    {
        if (RR::utility::random(100) <= 1) // 0 or 1, makes 1/50 chance
        {
            user.tools.erase("Fishing Rod");
            co_return dppcmd::command_result::from_error(std::format(Responses::ToolBroke, "Fishing Rod"));
        }

        numCaught *= 1.2;
    }

    long double cashGained = numCaught * fishPair.second;
    long double totalCash = user.cash + cashGained;

    if (RR::utility::random(100) < Constants::FishCoconutOdds)
        ItemSystem::giveCollectible("Coconut", context, user);

    user.mergeStats({
        { "Tasks Done", "1" },
        { "Money Gained from Tasks", RR::utility::cash2str(cashGained) }
    });

    user.setCash(member.value(), totalCash, cluster, context,
        std::format(Responses::TaskComplete, "caught", numCaught, fishPair.first, "rod",
            RR::utility::cash2str(cashGained), RR::utility::cash2str(totalCash)));
    user.modCooldown(user.fishCooldown = Constants::FishCooldown, member.value());
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}

dpp::task<dppcmd::command_result> Tasks::mine()
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    std::string toolName = ItemSystem::getBestTool(std::ranges::to<std::vector>(user.tools), "Pickaxe");
    const Tool* tool = dynamic_cast<const Tool*>(ItemSystem::getItem(toolName));
    int numMined = RR::utility::random(32, 65);

    if (user.perks.contains("Enchanter"))
    {
        if (RR::utility::random(100) <= 1) // 0 or 1, makes 1/50 chance
        {
            user.tools.erase(toolName);
            co_return dppcmd::command_result::from_error(std::format(Responses::ToolBroke, toolName));
        }

        numMined *= 1.2;
    }

    std::string mineral;
    switch (tool->tier())
    {
    case Tool::Tier::Stone:
        mineral = "iron";
        break;
    case Tool::Tier::Iron:
        mineral = "diamonds";
        break;
    case Tool::Tier::Diamond:
        mineral = "obsidian";
        break;
    case Tool::Tier::Netherite:
        mineral = "reinforced deepslate";
        break;
    default:
        mineral = "stone";
        break;
    }

    long double cashGained = numMined * 4 * tool->mult();
    long double totalCash = user.cash + cashGained;

    user.mergeStats({
        { "Tasks Done", "1" },
        { "Money Gained from Tasks", RR::utility::cash2str(cashGained) }
    });

    user.setCash(member.value(), totalCash, cluster, context,
        std::format(Responses::TaskComplete, "mined", numMined, mineral, toolName,
            RR::utility::cash2str(cashGained), RR::utility::cash2str(totalCash)));
    user.modCooldown(user.mineCooldown = Constants::MineCooldown, member.value());
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}

dpp::task<dppcmd::command_result> Tasks::genericTask(std::string_view toolType, std::string_view activity,
    std::string_view mineral, std::string_view cooldownKey, int64_t duration)
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    std::string toolName = ItemSystem::getBestTool(std::ranges::to<std::vector>(user.tools), toolType);
    const Tool* tool = dynamic_cast<const Tool*>(ItemSystem::getItem(toolName));

    int numMined;
    switch (tool->tier())
    {
    case Tool::Tier::Stone:
        numMined = RR::utility::random(Constants::GenericTaskStoneMin, Constants::GenericTaskStoneMax);
        break;
    case Tool::Tier::Iron:
        numMined = RR::utility::random(Constants::GenericTaskIronMin, Constants::GenericTaskIronMax);
        break;
    case Tool::Tier::Diamond:
        numMined = RR::utility::random(Constants::GenericTaskDiamondMin, Constants::GenericTaskDiamondMax);
        break;
    case Tool::Tier::Netherite:
        numMined = RR::utility::random(Constants::GenericTaskNetheriteMin, Constants::GenericTaskNetheriteMax);
        break;
    default:
        numMined = RR::utility::random(Constants::GenericTaskWoodMin, Constants::GenericTaskWoodMax);
        break;
    }

    if (user.perks.contains("Enchanter"))
    {
        if (RR::utility::random(100) <= 1) // 0 or 1, makes 50% chance
        {
            user.tools.erase(toolName);
            co_return dppcmd::command_result::from_error(std::format(Responses::ToolBroke, toolName));
        }

        numMined *= 1.2;
    }

    long double cashGained = numMined * 2.5L;
    long double totalCash = user.cash + cashGained;

    user.mergeStats({
        { "Tasks Done", "1" },
        { "Money Gained from Tasks", RR::utility::cash2str(cashGained) }
    });

    user.setCash(member.value(), totalCash, cluster, context,
        std::format(Responses::TaskComplete, activity, numMined, mineral, toolName,
            RR::utility::cash2str(cashGained), RR::utility::cash2str(totalCash)));

    std::unordered_map<std::string, int64_t&, string_hash, std::equal_to<>> cooldownMap = user.constructCooldownMap();
    if (auto it = cooldownMap.find(cooldownKey); it != cooldownMap.end())
        user.modCooldown(it->second = duration, member.value());

    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success();
}
