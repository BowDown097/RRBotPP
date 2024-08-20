#pragma once
#include "dpp-command-handler/modules/module.h"

class Prestige : public dpp::module<Prestige>
{
public:
    Prestige();
private:
    dpp::task<dpp::command_result> doPrestige();
    dpp::command_result prestigeInfo();
};
