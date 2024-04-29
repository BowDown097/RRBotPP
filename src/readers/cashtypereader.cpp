#include "cashtypereader.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "dpp-command-handler/utils/strings.h"
#include <dpp/dispatcher.h>

dpp::type_reader_result cash_in::read(dpp::cluster* cluster, const dpp::message_create_t* context, std::string_view input)
{
    if (input.size() > 1 && tolower(input.back()) == 'k')
    {
        if (long double value = dpp::utility::lexical_cast<long double>(input.substr(0, input.size() - 1)))
            add_result(value * 1000.0L);
    }
    else if (dpp::utility::iequals(input, "all"))
    {
        DbUser dbUser = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
        add_result(dbUser.cash);
    }
    else if (long double value = dpp::utility::lexical_cast<long double>(input))
    {
        add_result(value);
    }

    if (has_result())
        return dpp::type_reader_result::from_success();
    return dpp::type_reader_result::from_error(dpp::command_error::object_not_found, "Invalid input.");
}
