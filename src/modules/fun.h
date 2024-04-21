#ifndef FUN_H
#define FUN_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/usertypereader.h"

class Fun : public ModuleBase
{
public:
    Fun();
    MODULE_SETUP(Fun)
private:
    dpp::task<CommandResult> cat();
    dpp::task<CommandResult> dog();
    CommandResult flip();
    CommandResult gay(const std::optional<UserTypeReader>& userOpt);
    dpp::task<CommandResult> godword(const std::optional<int>& amountOpt);
    CommandResult magicConch(const std::string&);
    CommandResult penis(const std::optional<UserTypeReader>& userOpt);
    CommandResult sneed();
    dpp::task<CommandResult> terryQuote();
    CommandResult waifu();
};

#endif // FUN_H
