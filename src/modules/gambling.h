#pragma once
#include "dpp-command-handler/modules/module.h"

namespace dpp { class guild_member; }

class Gambling : public dpp::module<Gambling>
{
public:
    Gambling();
private:
    dpp::task<dpp::command_result> bet(const dpp::guild_member& member, long double bet, int number);
    dpp::task<dpp::command_result> dice(long double bet, int number);
    dpp::task<dpp::command_result> doubleGamble();
    dpp::task<dpp::command_result> pot(const std::optional<long double>& betOpt);
    dpp::task<dpp::command_result> roll55(long double bet);
    dpp::task<dpp::command_result> roll6969(long double bet);
    dpp::task<dpp::command_result> roll75(long double bet);
    dpp::task<dpp::command_result> roll99(long double bet);

    dpp::task<dpp::command_result> genericGamble(long double bet, long double target, long double mult,
                                                 bool exactRoll = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};
