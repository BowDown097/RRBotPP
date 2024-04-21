#ifndef CONFIG_H
#define CONFIG_H
#include "dpp-command-handler/module.h"
#include "dpp-command-handler/readers/channeltypereader.h"
#include "dpp-command-handler/readers/roletypereader.h"

class Config : public ModuleBase
{
public:
    Config();
    MODULE_SETUP(Config)
private:
    CommandResult addRank(int level, long double cost, const RoleTypeReader& roleRead);
    CommandResult clearConfig();
    CommandResult currentConfig();
    CommandResult disableCommand(const std::string& cmd);
    CommandResult disableFiltersInChannel(const ChannelTypeReader& channelRead);
    CommandResult disableModule(const std::string& module);
    CommandResult enableCommand(const std::string& cmd);
    CommandResult enableModule(const std::string& module);
    CommandResult setAdminRole(const RoleTypeReader& roleRead);
    CommandResult setDjRole(const RoleTypeReader& roleRead);
    CommandResult setLogsChannel(const ChannelTypeReader& channelRead);
    CommandResult setModRole(const RoleTypeReader& roleRead);
    CommandResult setPotChannel(const ChannelTypeReader& channelRead);
    CommandResult toggleDrops();
    CommandResult toggleInviteFilter();
    CommandResult toggleNsfw();
    CommandResult toggleScamFilter();
    CommandResult unwhitelistChannel(const ChannelTypeReader& channelRead);
    CommandResult whitelistChannel(const ChannelTypeReader& channelRead);
};

#endif // CONFIG_H
