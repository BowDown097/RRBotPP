#ifndef ITEM_H
#define ITEM_H
#include <string_view>

class Item
{
public:
    std::string_view name() const { return m_name; }
    long double price() const { return m_price; }
protected:
    std::string_view m_name;
    long double m_price;

    constexpr Item(std::string_view name, long double price) : m_name(name), m_price(price) {}
};

#endif // ITEM_H
