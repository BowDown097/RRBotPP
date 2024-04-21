#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H
#include <string_view>

class Achievement
{
public:
    constexpr Achievement(std::string_view name, std::string_view description, long double reward = 0)
        : m_description(description), m_name(name), m_reward(reward) {}

    std::string_view description() const { return m_description; }
    std::string_view name() const { return m_name; }
    long double reward() const { return m_reward; }
private:
    std::string_view m_description;
    std::string_view m_name;
    long double m_reward;
};

#endif // ACHIEVEMENT_H
