#pragma once
#include <array>
#include "entities/achievement.h"
#include "entities/goods/tool.h"

namespace Constants
{
    // ********************
    //     ACHIEVEMENTS
    // ********************
    constexpr std::array DefaultAchievements = {
        Achievement("Hard Boiled Egg", "Find a hard boiled egg.", 1337),
        Achievement("I Just Feel Bad", "Take a million dollar L."),
        Achievement("Jackpot!", "Get a jackpot with $slots."),
        Achievement("Luckiest Dude Alive", "Win $69.69."),
        Achievement("Literally 1984", "Get muted."),
        Achievement("Maxed!", "Reach the max prestige.", 1420),
        Achievement("OH BABY A TRIPLE", "Roll triples in $dice."),
        Achievement("Prestiged!", "Get your first prestige.", 1000),
        Achievement("Pretty Damn Lucky", "Win $99+.")
    };
    constexpr std::array GamblingAchievementNames = {
        "I Just Feel Bad",
        "Jackpot!",
        "Luckiest Dude Alive",
        "OH BABY A TRIPLE",
        "Pretty Damn Lucky"
    };
    // ********************
    //     CASH SYSTEM
    // ********************
    constexpr long double CashMultiplier = 1.0L;
    constexpr long double TransactionMin = 100;
    // ********************
    //      COOLDOWNS
    // ********************
    constexpr long BullyCooldown = 600;
    constexpr long ChopCooldown = 3600;
    constexpr long DailyCooldown = 86400;
    constexpr long DealCooldown = 3600;
    constexpr long DigCooldown = 3600;
    constexpr long FarmCooldown = 3600;
    constexpr long FishCooldown = 3600;
    constexpr long HackCooldown = 3600;
    constexpr long HuntCooldown = 3600;
    constexpr long LootCooldown = 3600;
    constexpr long MessageCashCooldown = 60;
    constexpr long MineCooldown = 3600;
    constexpr long PrestigeCooldown = 43200;
    constexpr long RapeCooldown = 3600;
    constexpr long RobCooldown = 3600;
    constexpr long ScavengeCooldown = 60;
    constexpr long ShootCooldown = 14400;
    constexpr long SlaveryCooldown = 3600;
    constexpr long WhoreCooldown = 3600;
    // ********************
    //        CRIME
    // ********************
    constexpr double GenericCrimeWinOdds = 80;
    constexpr long double GenericCrimeLossMax = 461;
    constexpr long double GenericCrimeLossMin = 69;
    constexpr double GenericCrimeToolOdds = 4;
    constexpr long double GenericCrimeWinMax = 691;
    constexpr long double GenericCrimeWinMin = 69;
    constexpr double HackOdds = 10;
    constexpr double RapeOdds = 50;
    constexpr long double RapeMaxPercent = 9;
    constexpr long double RapeMinPercent = 5;
    constexpr double RobOdds = 40;
    constexpr long double RobMaxPercent = 20;
    constexpr long double RobMinCash = 100;
    constexpr long double ScavengeMinCash = 50;
    constexpr long double ScavengeMaxCash = 100;
    constexpr double ScavengeTimeout = 15;
    // ********************
    //       GAMBLING
    // ********************
    constexpr int DoubleOdds = 45;
    // ********************
    //        GANGS
    // ********************
    constexpr long double GangCreationCost = 5000;
    constexpr int GangMaxMembers = 10;
    constexpr long double GangRenameCost = 2000;
    constexpr long double GangVaultCost = 5000;
    constexpr std::array<std::string_view, 3> GangPositions = { "Leader", "Elder", "Member" };
    constexpr int MaxGangsPerGuild = 50;
    constexpr long double VaultTaxPercent = 1.5L;
    // ********************
    //     INVESTMENTS
    // ********************
    constexpr long double InvestmentFeePercent = 1.5L;
    constexpr long double InvestmentMinAmount = 0.01L;
    // ********************
    //        TASKS
    // ********************
    constexpr std::array Fish = {
        std::pair { "carp", 24 },
        std::pair { "trout", 27 },
        std::pair { "goldfish", 30 }
    };
    constexpr double FishCoconutOdds = 20;
    constexpr int GenericTaskWoodMax = 65;
    constexpr int GenericTaskWoodMin = 32;
    constexpr int GenericTaskStoneMax = 113;
    constexpr int GenericTaskStoneMin = 65;
    constexpr int GenericTaskIronMax = 161;
    constexpr int GenericTaskIronMin = 113;
    constexpr int GenericTaskDiamondMax = 209;
    constexpr int GenericTaskDiamondMin = 161;
    constexpr int GenericTaskNetheriteMax = 257;
    constexpr int GenericTaskNetheriteMin = 209;
    // ********************
    //        ITEMS
    // ********************
    constexpr std::array Tools = {
        Tool("Wooden Pickaxe", 4500),
        Tool("Stone Pickaxe", 6000, 0, 0, 1.33L),
        Tool("Iron Pickaxe", 7500, 0, 0, 1.66L),
        Tool("Diamond Pickaxe", 9000, 0, 0, 2),
        Tool("Netherite Pickaxe", 10500, 0, 0, 2.33L),
        Tool("Wooden Sword", 4500, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Sword", 6000, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Sword", 7500, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Sword", 9000, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Sword", 10500, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Shovel", 4500, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Shovel", 6000, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Shovel", 7500, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Shovel", 9000, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Shovel", 10500, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Axe", 4500, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Axe", 6000, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Axe", 7500, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Axe", 9000, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Axe", 10500, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Hoe", 4500, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Hoe", 6000, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Hoe", 7500, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Hoe", 9000, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Hoe", 10500, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Fishing Rod", 7500, Fish.front().second * 7L, Fish.back().second * 15L)
    };
    // ********************
    //      PRESTIGE
    // ********************
    constexpr int MaxPrestige = 10;
}
