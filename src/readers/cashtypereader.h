#pragma once
#include "dpp-command-handler/readers/typereader.h"

struct cash_in : dpp::type_reader<long double>
{
    dpp::type_reader_result read(dpp::cluster* cluster, const dpp::message_create_t* context,
                                 std::string_view input) override;
};
