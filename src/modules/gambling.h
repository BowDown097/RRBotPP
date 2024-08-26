#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Gambling : public dppcmd::module<Gambling>
{
public:
    Gambling();
private:
    dpp::task<dppcmd::command_result> bet(const dpp::guild_member& member, long double bet, int number);
    dpp::task<dppcmd::command_result> dice(long double bet, int number);
    dpp::task<dppcmd::command_result> doubleGamble();
    dpp::task<dppcmd::command_result> pot(const std::optional<long double>& betOpt);
    dpp::task<dppcmd::command_result> roll55(long double bet);
    dpp::task<dppcmd::command_result> roll6969(long double bet);
    dpp::task<dppcmd::command_result> roll75(long double bet);
    dpp::task<dppcmd::command_result> roll99(long double bet);

    dpp::task<dppcmd::command_result> genericGamble(long double bet, long double target, long double mult,
                                                    bool exactRoll = false);
    void statUpdate(class DbUser& user, bool success, long double gain);
};
