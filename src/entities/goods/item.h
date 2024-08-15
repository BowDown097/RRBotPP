#pragma once
#include <string_view>

class Item
{
public:
    virtual ~Item() = default;

    constexpr std::string_view name() const { return m_name; }
    constexpr long double worth() const { return m_worth; }

    friend constexpr bool operator==(const Item& lhs, const Item& rhs) { return lhs.m_name == rhs.m_name; }
protected:
    std::string_view m_name;
    long double m_worth;

    consteval Item(std::string_view name, long double worth) : m_name(name), m_worth(worth) {}
};
