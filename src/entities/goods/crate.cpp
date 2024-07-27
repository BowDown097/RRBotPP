#include "crate.h"
#include "data/constants.h"
#include "database/entities/dbuser.h"
#include "utils/random.h"

std::vector<const Item*> Crate::open(const DbUser& user) const
{
    std::vector<const Item*> items;

    if (m_tier > Tier::Daily)
    {
        if (int ammoRoll = RR::utility::random(tierValue() + 1))
        {
            auto checkAvailable = [ammoRoll](const Ammo& a) { return a.crateMultiplier() * ammoRoll >= 1; };
            const Ammo& randomAmmo = RR::utility::randomElement(Constants::Ammos | std::views::filter(checkAvailable));
            // the use of addressof here avoids a weird false positive some static code analyzers will throw
            items.insert(items.cend(), randomAmmo.crateMultiplier() * ammoRoll, std::addressof(randomAmmo));
        }
    }

    for (int i = 0; i < m_consumableCount; ++i)
        items.push_back(&RR::utility::randomElement(Constants::Consumables));

    if (m_toolCount > 0)
    {
        if (m_tier >= Tier::Diamond)
        {
            for (int i = 0; i < m_toolCount; ++i)
                items.push_back(&RR::utility::randomElement(Constants::Tools));
        }
        else
        {
            auto notNetherite = [](const Tool& t) { return t.tier() < Tool::Tier::Netherite; };
            for (int i = 0; i < m_toolCount; ++i)
                items.push_back(&RR::utility::randomElement(Constants::Tools | std::views::filter(notNetherite)));
        }
    }

    int weaponRolls = RR::utility::random(tierValue() + 1);
    for (int i = 0; i < weaponRolls; ++i)
    {
        int weaponDropRoll = RR::utility::random(100);
        auto availableWeapons = Constants::Weapons | std::views::filter([this, weaponDropRoll](const Weapon& w) {
            return weaponDropRoll < w.dropChance() && std::ranges::contains(w.insideCrates(), m_name);
        });

        if (!std::ranges::empty(availableWeapons))
        {
            items.push_back(&RR::utility::randomElement(availableWeapons));
            break;
        }
    }

    size_t dupeTools = std::erase_if(items, [&user](const Item* i) { return user.tools.contains(i->name()); });
    for (int i = 0; i < dupeTools * 3; ++i)
        items.push_back(&RR::utility::randomElement(Constants::Consumables));

    return items;
}
