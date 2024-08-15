#pragma once
#include "item.h"
#include <vector>

class DbUser;

struct CrateDrop
{
    std::vector<const Item*> items;
    long double refund{};
};

class Crate : public Item
{
public:
    enum class Tier { Daily, Bronze, Silver, Gold, Diamond };
    consteval Crate(std::string_view name, long double worth, Tier tier, int consumableCount = 0, int toolCount = 0,
                    long double cash = 0)
        : Item(name, worth), m_cash(cash), m_consumableCount(consumableCount), m_tier(tier), m_toolCount(toolCount) {}

    friend constexpr auto operator<=>(Tier lhs, Tier rhs) noexcept
    { return static_cast<int>(lhs) <=> static_cast<int>(rhs); }

    constexpr long double cash() const { return m_cash; }
    constexpr int consumableCount() const { return m_consumableCount; }
    constexpr Tier tier() const { return m_tier; }
    constexpr int tierValue() const { return static_cast<int>(m_tier); }
    constexpr int toolCount() const { return m_toolCount; }

    CrateDrop open(const DbUser& user) const;
private:
    long double m_cash;
    int m_consumableCount;
    Tier m_tier;
    int m_toolCount;
};
