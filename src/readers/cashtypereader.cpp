#include "cashtypereader.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/utils/lexical_cast.h"
#include "dppcmd/utils/strings.h"
#include <dpp/dispatcher.h>

dppcmd::type_reader_result RR::cash_in::read(dpp::cluster* cluster, const dpp::message_create_t* context, std::string_view input)
{
    if (input.size() > 1 && std::tolower(input.back()) == 'k')
        add_result(dppcmd::utility::lexical_cast<long double>(input.substr(0, input.size() - 1)) * 1000.0L);
    else if (dppcmd::utility::iequals(input, "all"))
        add_result(MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id).cash);
    else
        add_result(dppcmd::utility::lexical_cast<long double>(input));

    // execution will not reach this point if any of the above casts have failed,
    // so we can just return a success result here.
    return dppcmd::type_reader_result::from_success();
}
