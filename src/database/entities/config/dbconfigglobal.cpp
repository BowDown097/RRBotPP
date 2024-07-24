#include "dbconfigglobal.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

DbConfigGlobal::DbConfigGlobal(bsoncxx::document::view doc)
{
    bsoncxx_elem_to_array(doc["bannedUsers"], bannedUsers, int64);
    bsoncxx_elem_to_array(doc["disabledCommands"], disabledCommands, string);
}

bsoncxx::document::value DbConfigGlobal::toDocument() const
{
    bsoncxx::builder::stream::array bannedUsersArr;
    bsoncxx_stream_array_into(bannedUsers, bannedUsersArr);

    bsoncxx::builder::stream::array disabledCommandsArr;
    bsoncxx_stream_array_into(disabledCommands, disabledCommandsArr);

    return bsoncxx::builder::stream::document()
           << "bannedUsers" << bannedUsersArr
           << "disabledCommands" << disabledCommandsArr
           << bsoncxx::builder::stream::finalize;
}
