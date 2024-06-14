#pragma once
#include "item.h"
#include <vector>

class DbUser;

class Crate : public Item
{
public:
    enum class Tier { Daily, Bronze, Silver, Gold, Diamond };
    consteval Crate(std::string_view name, long double price, Tier tier, int consumableCount = 0, int toolCount = 0,
                    long double cash = 0)
        : Item(name, price), m_cash(cash), m_consumableCount(consumableCount), m_tier(tier), m_toolCount(toolCount) {}

    constexpr long double cash() const { return m_cash; }
    constexpr int consumableCount() const { return m_consumableCount; }
    constexpr Tier tier() const { return m_tier; }
    constexpr int tierValue() const { return static_cast<int>(m_tier); }
    constexpr int toolCount() const { return m_toolCount; }

    std::vector<Item> open(const DbUser& user) const;
private:
    long double m_cash;
    int m_consumableCount;
    Tier m_tier;
    int m_toolCount;
};
