#ifndef TOOL_H
#define TOOL_H
#include "item.h"

class Tool : public Item
{
public:
    constexpr Tool(std::string_view name, long double price, long double genericMin = 0,
                   long double genericMax = 0, long double mult = 1)
        : Item(name, price), m_genericMin(genericMin), m_genericMax(genericMax), m_mult(mult) {}

    long double genericMin() const { return m_genericMin; }
    long double genericMax() const { return m_genericMax; }
    long double mult() const { return m_mult; }
private:
    long double m_genericMin;
    long double m_genericMax;
    long double m_mult;
};

#endif // TOOL_H
