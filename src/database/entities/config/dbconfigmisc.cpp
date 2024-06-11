#include "dbconfigmisc.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigMisc::DbConfigMisc(const bsoncxx::document::value& doc)
{
    dropsDisabled = bsoncxx_get_or_default(doc["dropsDisabled"], bool);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    inviteFilterEnabled = bsoncxx_get_or_default(doc["inviteFilterEnabled"], bool);
    nsfwEnabled = bsoncxx_get_or_default(doc["nsfwEnabled"], bool);
    scamFilterEnabled = bsoncxx_get_or_default(doc["scamFilterEnabled"], bool);

    bsoncxx::array::view disabledCommandsArr = bsoncxx_get_or_default(doc["disabledCommands"], array);
    for (auto it = disabledCommandsArr.cbegin(); it != disabledCommandsArr.cend(); ++it)
        disabledCommands.push_back(std::string(it->get_string()));

    bsoncxx::array::view disabledModulesArr = bsoncxx_get_or_default(doc["disabledModules"], array);
    for (auto it = disabledModulesArr.cbegin(); it != disabledModulesArr.cend(); ++it)
        disabledModules.push_back(std::string(it->get_string()));

    bsoncxx::array::view filteredTermsArr = bsoncxx_get_or_default(doc["filteredTerms"], array);
    for (auto it = filteredTermsArr.cbegin(); it != filteredTermsArr.cend(); ++it)
        filteredTerms.push_back(std::string(it->get_string()));
}

bsoncxx::document::value DbConfigMisc::toDocument() const
{
    bsoncxx::builder::stream::array disabledCommandsArr;
    for (const std::string& disabledCommand : disabledCommands)
        disabledCommandsArr << disabledCommand;

    bsoncxx::builder::stream::array disabledModulesArr;
    for (const std::string& disabledModule : disabledModules)
        disabledModulesArr << disabledModule;

    bsoncxx::builder::stream::array filteredTermsArr;
    for (const std::string& filteredTerm : filteredTerms)
        filteredTermsArr << filteredTerm;

    return bsoncxx::builder::stream::document()
           << "disabledCommands" << disabledCommandsArr
           << "disabledModules" << disabledModulesArr
           << "dropsDisabled" << dropsDisabled
           << "filteredTerms" << filteredTermsArr
           << "guildId" << guildId
           << "inviteFilterEnabled" << inviteFilterEnabled
           << "nsfwEnabled" << nsfwEnabled
           << "scamFilterEnabled" << scamFilterEnabled
           << bsoncxx::builder::stream::finalize;
}
