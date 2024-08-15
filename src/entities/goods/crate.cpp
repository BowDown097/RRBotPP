#include "crate.h"
#include "data/constants.h"
#include "database/entities/dbuser.h"
#include "utils/random.h"

CrateDrop Crate::open(const DbUser& user) const
{
    CrateDrop drop;

    // -AMMO DROP-
    // rolls a random number from 0 - tierValue + 1
    // gets ammo types whose crate multiplier * roll >= 1
    // if there are any, grabs a random ammo type from that pool
    // gives a crate multiplier * roll amount of that ammo type
    if (m_tier > Tier::Daily)
    {
        if (int ammoRoll = RR::utility::random(tierValue() + 1))
        {
            auto checkAvailable = [ammoRoll](const Ammo& a) { return a.crateMultiplier() * ammoRoll >= 1; };
            auto availableAmmo = Constants::Ammos | std::views::filter(checkAvailable);
            if (!std::ranges::empty(availableAmmo))
            {
                const Ammo& randomAmmo = RR::utility::randomElement(Constants::Ammos | std::views::filter(checkAvailable));
                drop.items.insert(drop.items.cend(), randomAmmo.crateMultiplier() * ammoRoll, &randomAmmo);
            }
        }
    }

    // -CONSUMABLE DROP-
    // m_consumableCount times, rolls a random consumable and gives it
    for (int i = 0; i < m_consumableCount; ++i)
        drop.items.push_back(&RR::utility::randomElement(Constants::Consumables));

    // -TOOL DROP-
    // m_toolCount times, rolls a random tool
    // if the tool is unique, gives it, otherwise adds 80% of the tool's worth to the refund
    // netherite tools are excluded unless if the crate is diamond or higher
    if (m_toolCount > 0)
    {
        auto notNetherite = [](const Tool& t) { return t.tier() < Tool::Tier::Netherite; };
        std::set<std::string, std::less<>> usedTools = user.tools;
        for (int i = 0; i < m_toolCount; ++i)
        {
            const Tool& tool = m_tier >= Tier::Diamond
                ? RR::utility::randomElement(Constants::Tools)
                : RR::utility::randomElement(Constants::Tools | std::views::filter(notNetherite));
            if (usedTools.emplace(tool.name()).second)
                drop.items.push_back(&tool);
            else
                drop.refund += tool.worth() * 0.8L;
        }
    }

    // -WEAPON DROP-
    // rolls a random number from 0 - tierValue + 1, and that many times...
    // rolls a random number from 0-99, and gets weapons that are
    // inside the current crate and whose drop chance falls within the roll
    // if there are any, pulls a random weapon from that pool
    // if the weapon is unique, gives it, otherwise adds 80% of the weapon's worth to the refund
    int weaponRolls = RR::utility::random(tierValue() + 1);
    for (int i = 0; i < weaponRolls; ++i)
    {
        int weaponDropRoll = RR::utility::random(100);
        auto availableWeapons = Constants::Weapons | std::views::filter([this, weaponDropRoll](const Weapon& w) {
            return weaponDropRoll < w.dropChance() && std::ranges::contains(w.insideCrates(), m_name);
        });
        if (std::ranges::empty(availableWeapons))
            continue;

        const Weapon& weapon = RR::utility::randomElement(availableWeapons);
        if (!user.weapons.contains(weapon.name()))
            drop.items.push_back(&weapon);
        else
            drop.refund += weapon.worth() * 0.8L;
        break;
    }

    return drop;
}
