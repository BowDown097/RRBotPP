#ifndef GAMBLING_H
#define GAMBLING_H
#include "dpp-command-handler/module.h"

class Gambling : public dpp::module_base
{
public:
    Gambling();
    MODULE_SETUP(Gambling)
private:
    dpp::task<dpp::command_result> dice(long double bet, int number);
    dpp::task<dpp::command_result> doubleGamble();
    dpp::task<dpp::command_result> pot(const std::optional<long double>& bet);
    dpp::task<dpp::command_result> roll55(long double bet);
    dpp::task<dpp::command_result> roll6969(long double bet);
    dpp::task<dpp::command_result> roll75(long double bet);
    dpp::task<dpp::command_result> roll99(long double bet);

    dpp::task<dpp::command_result> genericGamble(long double bet, long double odds, long double mult,
                                                 bool exactRoll = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};

#endif // GAMBLING_H
