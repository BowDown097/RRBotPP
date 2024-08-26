#pragma once
#include "dppcmd/modules/module.h"

class Prestige : public dppcmd::module<Prestige>
{
public:
    Prestige();
private:
    dpp::task<dppcmd::command_result> doPrestige();
    dppcmd::command_result prestigeInfo();
};
