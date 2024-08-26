#pragma once
#include "dppcmd/readers/typereader.h"

namespace RR
{
    struct cash_in : dppcmd::type_reader<long double>
    {
        dppcmd::type_reader_result read(dpp::cluster* cluster, const dpp::message_create_t* context,
                                        std::string_view input) override;
    };
}
