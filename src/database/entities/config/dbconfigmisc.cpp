#include "dbconfigmisc.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigMisc::DbConfigMisc(const bsoncxx::document::value& doc)
{
    dropsDisabled = doc["dropsDisabled"].get_bool();
    guildId = doc["guildId"].get_int64();
    inviteFilterEnabled = doc["inviteFilterEnabled"].get_bool();
    nsfwEnabled = doc["nsfwEnabled"].get_bool();
    scamFilterEnabled = doc["scamFilterEnabled"].get_bool();

    bsoncxx::array::view disabledCommandsArr = doc["disabledCommands"].get_array();
    for (auto it = disabledCommandsArr.cbegin(); it != disabledCommandsArr.cend(); ++it)
        disabledCommands.push_back(std::string(it->get_string()));

    bsoncxx::array::view disabledModulesArr = doc["disabledModules"].get_array();
    for (auto it = disabledModulesArr.cbegin(); it != disabledModulesArr.cend(); ++it)
        disabledModules.push_back(std::string(it->get_string()));
}

bsoncxx::document::value DbConfigMisc::toDocument() const
{
    bsoncxx::builder::stream::array disabledCommandsArr;
    for (const std::string& disabledCommand : disabledCommands)
        disabledCommandsArr << disabledCommand;

    bsoncxx::builder::stream::array disabledModulesArr;
    for (const std::string& disabledModule : disabledModules)
        disabledModulesArr << disabledModule;

    return bsoncxx::builder::stream::document()
           << "disabledCommands" << disabledCommandsArr
           << "disabledModules" << disabledModulesArr
           << "dropsDisabled" << dropsDisabled
           << "guildId" << guildId
           << "inviteFilterEnabled" << inviteFilterEnabled
           << "nsfwEnabled" << nsfwEnabled
           << "scamFilterEnabled" << scamFilterEnabled
           << bsoncxx::builder::stream::finalize;
}
