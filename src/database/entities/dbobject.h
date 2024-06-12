#pragma once
#include <bsoncxx/document/value.hpp>

#define bsoncxx_get_or_default(elem, target) \
    elem ? elem.get_##target() : decltype(elem.get_##target()){}
#define bsoncxx_get_value_or_default(elem, target) \
    elem ? elem.get_##target().value : decltype(elem.get_##target().value){}

#define bsoncxx_elem_to_array(elem, outArray, type) \
    bsoncxx::array::view outArray##Arr = bsoncxx_get_or_default(elem, array); \
    for (auto it = outArray##Arr.cbegin(); it != outArray##Arr.cend(); ++it) \
        outArray.emplace_back(it->get_##type());

#define bsoncxx_elem_to_map(elem, map, valType) \
    bsoncxx::document::view map##Doc = bsoncxx_get_or_default(elem, document); \
    for (auto it = map##Doc.cbegin(); it != map##Doc.cend(); ++it) \
        map.emplace(dpp::utility::lexical_cast<decltype(map)::key_type>(it->key()), it->get_##valType());

#define bsoncxx_stream_map_into(map, doc) \
    for (const auto& [key, val] : map) \
        doc << dpp::utility::lexical_cast<std::string>(key) << val;

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};
