#include "crate.h"
#include "data/constants.h"
#include "database/entities/dbuser.h"
#include "utils/random.h"

std::vector<Item> Crate::open(const DbUser& user) const
{
    std::vector<Item> items;

    if (tierValue() > static_cast<int>(Tier::Daily))
    {
        if (int ammoRoll = RR::utility::random(tierValue() + 1))
        {
            Ammo randomAmmo = RR::utility::randomElement(Constants::Ammos | std::views::filter([ammoRoll](const Ammo& a) {
                return a.crateMultiplier() * ammoRoll >= 1;
            }));
            items.insert(items.cend(), randomAmmo.crateMultiplier() * ammoRoll, randomAmmo);
        }
    }

    for (int i = 0; i < m_consumableCount; ++i)
        items.push_back(RR::utility::randomElement(Constants::Consumables));

    if (m_toolCount > 0)
    {
        if (tierValue() < static_cast<int>(Tier::Diamond))
        {
            auto availableTools = Constants::Tools | std::views::filter([](const Tool& t) {
                return !t.name().starts_with("Netherite");
            });
            for (int i = 0; i < m_toolCount; ++i)
                items.push_back(RR::utility::randomElement(availableTools));
        }
        else
        {
            for (int i = 0; i < m_toolCount; ++i)
                items.push_back(RR::utility::randomElement(Constants::Tools));
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
            items.push_back(RR::utility::randomElement(availableWeapons));
            break;
        }
    }

    size_t dupeTools = std::erase_if(items, [&user](const Item& i) { return std::ranges::contains(user.tools, i.name()); });
    for (int i = 0; i < dupeTools * 3; ++i)
        items.push_back(RR::utility::randomElement(Constants::Consumables));

    return items;
}
