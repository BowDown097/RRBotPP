#include "dbconfigmisc.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigMisc::DbConfigMisc(bsoncxx::document::view doc)
{
    dropsDisabled = bsoncxx_get_or_default(doc["dropsDisabled"], bool);
    guildId = bsoncxx_get_or_default(doc["guildId"], int64);
    inviteFilterEnabled = bsoncxx_get_or_default(doc["inviteFilterEnabled"], bool);
    nsfwEnabled = bsoncxx_get_or_default(doc["nsfwEnabled"], bool);
    scamFilterEnabled = bsoncxx_get_or_default(doc["scamFilterEnabled"], bool);
    bsoncxx_elem_to_array(doc["disabledCommands"], disabledCommands, string);
    bsoncxx_elem_to_array(doc["disabledModules"], disabledModules, string);
    bsoncxx_elem_to_array(doc["filteredTerms"], filteredTerms, string);
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
