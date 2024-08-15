#pragma once
#include "item.h"

class Perk : public Item
{
public:
    consteval Perk(std::string_view name, long double worth, std::string_view description, long duration)
        : Item(name, worth), m_description(description), m_duration(duration) {}

    constexpr std::string_view description() const { return m_description; }
    constexpr long duration() const { return m_duration; }
private:
    std::string_view m_description;
    long m_duration;
};
