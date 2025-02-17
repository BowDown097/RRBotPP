#pragma once
#include <string>

class Credentials
{
public:
    static Credentials& instance() { static Credentials c; return c; }
    void initialize();

    const std::string& mongoConnectionString() const { return m_mongoConnectionString; }
    const std::string& token() const { return m_token; }
private:
    std::string m_mongoConnectionString;
    std::string m_token;
};
