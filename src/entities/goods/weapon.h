#pragma once
#include "item.h"
#include <span>

class Weapon : public Item
{
public:
	enum class Type { Gun, Knife };

	consteval Weapon(std::string_view name, int accuracy, std::string_view ammo, int damageMin,
					 int damageMax, int dropChance, std::string_view information, Type type,
                     std::initializer_list<std::string_view> insideCrates)
        : Item(name, 0), m_accuracy(accuracy), m_ammo(ammo), m_damageMax(damageMax), m_damageMin(damageMin),
          m_dropChance(dropChance), m_information(information), m_type(type)
    {
        m_insideCrates.second = insideCrates.size();
        std::copy(insideCrates.begin(), insideCrates.end(), m_insideCrates.first.begin());
    }

    constexpr int accuracy() const { return m_accuracy; }
    constexpr std::string_view ammo() const { return m_ammo; }
    constexpr int damageMax() const { return m_damageMax; }
    constexpr int damageMin() const { return m_damageMin; }
    constexpr int dropChance() const { return m_dropChance; }
    constexpr std::string_view information() const { return m_information; }
    constexpr Type type() const { return m_type; }

    constexpr std::span<const std::string_view> insideCrates() const
    { return std::span(m_insideCrates.first.begin(), m_insideCrates.second); }

	constexpr std::string_view typeAsString() const
	{
		static constexpr std::array<std::string_view, 2> stringValues = { "Gun", "Knife" };
		return stringValues[static_cast<int>(m_type)];
	}
private:
    int m_accuracy;
    std::string_view m_ammo;
    int m_damageMax;
    int m_damageMin;
    int m_dropChance;
    std::string_view m_information;
    std::pair<std::array<std::string_view, 5>, size_t> m_insideCrates;
    Type m_type;
};
