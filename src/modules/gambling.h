#pragma once
#include "dpp-command-handler/module.h"
#include "readers/cashtypereader.h"

class Gambling : public dpp::module_base
{
public:
    Gambling();
    MODULE_SETUP(Gambling)
private:
    dpp::task<dpp::command_result> dice(const cash_in& betIn, int number);
    dpp::task<dpp::command_result> doubleGamble();
    dpp::task<dpp::command_result> pot(const std::optional<cash_in>& betIn);
    dpp::task<dpp::command_result> roll55(const cash_in& betIn);
    dpp::task<dpp::command_result> roll6969(const cash_in& betIn);
    dpp::task<dpp::command_result> roll75(const cash_in& betIn);
    dpp::task<dpp::command_result> roll99(const cash_in& betIn);

    dpp::task<dpp::command_result> genericGamble(long double bet, long double odds, long double mult,
                                                 bool exactRoll = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};
