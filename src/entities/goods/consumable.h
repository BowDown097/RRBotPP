#pragma once
#include "item.h"

class Consumable : public Item
{
public:
    consteval Consumable(std::string_view name, std::string_view information, std::string_view negEffect,
                         std::string_view posEffect, long duration, int max = -1)
        : Item(name, 0), m_duration(duration), m_information(information), m_max(max), m_negEffect(negEffect),
          m_posEffect(posEffect) {}

    constexpr long duration() const { return m_duration; }
    constexpr std::string_view information() const { return m_information; }
    constexpr int max() const { return m_max; }
    constexpr std::string_view negEffect() const { return m_negEffect; }
    constexpr std::string_view posEffect() const { return m_posEffect; }
private:
    long m_duration;
    std::string_view m_information;
    int m_max;
    std::string_view m_negEffect;
    std::string_view m_posEffect;
};
