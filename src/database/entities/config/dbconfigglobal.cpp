#include "dbconfigglobal.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigGlobal::DbConfigGlobal(bsoncxx::document::view doc)
{
    bsoncxx::array::view bannedUsersDoc = bsoncxx_get_or_default(doc["bannedUsers"], array);
    for (auto it = bannedUsersDoc.cbegin(); it != bannedUsersDoc.cend(); ++it)
        bannedUsers.push_back(it->get_int64());

    bsoncxx::array::view disabledCommandsDoc = bsoncxx_get_or_default(doc["disabledCommands"], array);
    for (auto it = disabledCommandsDoc.cbegin(); it != disabledCommandsDoc.cend(); ++it)
        disabledCommands.push_back(std::string(it->get_string()));
}

bsoncxx::document::value DbConfigGlobal::toDocument() const
{
    bsoncxx::builder::stream::array bannedUsersArr;
    for (int64_t userId : bannedUsers)
        bannedUsersArr << userId;

    bsoncxx::builder::stream::array disabledCommandsArr;
    for (const std::string& command : disabledCommands)
        disabledCommandsArr << command;

    return bsoncxx::builder::stream::document()
           << "bannedUsers" << bannedUsersArr
           << "disabledCommands" << disabledCommandsArr
           << bsoncxx::builder::stream::finalize;
}
