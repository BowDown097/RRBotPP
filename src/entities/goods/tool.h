#pragma once
#include "item.h"

class Tool : public Item
{
public:
    enum class Tier { Wood, Stone, Iron, Diamond, Netherite, Special };

    consteval Tool(std::string_view name, long double worth, Tier tier, long double genericMin = 0,
                   long double genericMax = 0, long double mult = 1)
        : Item(name, worth), m_genericMin(genericMin), m_genericMax(genericMax), m_mult(mult), m_tier(tier) {}

    friend constexpr auto operator<=>(Tier lhs, Tier rhs) noexcept
    { return static_cast<int>(lhs) <=> static_cast<int>(rhs); }

    constexpr long double genericMin() const { return m_genericMin; }
    constexpr long double genericMax() const { return m_genericMax; }
    constexpr long double mult() const { return m_mult; }
    constexpr Tier tier() const { return m_tier; }
private:
    long double m_genericMin;
    long double m_genericMax;
    long double m_mult;
    Tier m_tier;
};
