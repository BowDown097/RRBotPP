#pragma once
#include "item.h"

class Collectible : public Item
{
public:
    consteval Collectible(std::string_view name, long double worth, std::string_view description, std::string_view image,
                          bool discardable = true)
        : Item(name, worth), m_description(description), m_discardable(discardable), m_image(image) {}

    constexpr std::string_view description() const { return m_description; }
    constexpr bool discardable() const { return m_discardable; }
    constexpr std::string_view image() const { return m_image; }
private:
    std::string_view m_description;
    bool m_discardable;
    std::string_view m_image;
};
