#pragma once
#include <string_view>

class Item
{
public:
    virtual ~Item() = default;

    constexpr std::string_view name() const { return m_name; }
    constexpr long double price() const { return m_price; }

    friend constexpr bool operator==(const Item& lhs, const Item& rhs) { return lhs.m_name == rhs.m_name; }
protected:
    std::string_view m_name;
    long double m_price;

    consteval Item(std::string_view name, long double price) : m_name(name), m_price(price) {}
};
