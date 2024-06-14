#pragma once
#include "item.h"

class Ammo : public Item
{
public:
    consteval Ammo(std::string_view name, double crateMultiplier)
        : Item(name, 0), m_crateMultiplier(crateMultiplier) {}

    constexpr double crateMultiplier() const { return m_crateMultiplier; }
private:
    double m_crateMultiplier;
};
