#ifndef FUN_H
#define FUN_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Fun : public dpp::module_base
{
public:
    Fun();
    MODULE_SETUP(Fun)
private:
    dpp::task<dpp::command_result> cat();
    dpp::task<dpp::command_result> dog();
    dpp::command_result flip();
    dpp::command_result gay(const std::optional<dpp::user_in>& userOpt);
    dpp::task<dpp::command_result> godword(const std::optional<int>& amountOpt);
    dpp::command_result magicConch(const dpp::remainder<std::string>&);
    dpp::command_result penis(const std::optional<dpp::user_in>& userOpt);
    dpp::command_result sneed();
    dpp::task<dpp::command_result> terryQuote();
    dpp::command_result waifu();
};

#endif // FUN_H
