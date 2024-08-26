#pragma once
#include <bsoncxx/document/value.hpp>

#define bsoncxx_get_or_default(elem, target) \
    elem ? elem.get_##target() : decltype(elem.get_##target()){}
#define bsoncxx_get_value_or_default(elem, target) \
    elem ? elem.get_##target().value : decltype(elem.get_##target().value){}

#define bsoncxx_elem_to_map(elem, map, valType) \
    bsoncxx::document::view map##Doc = bsoncxx_get_or_default(elem, document); \
    for (auto it = map##Doc.cbegin(); it != map##Doc.cend(); ++it) \
        map.emplace(dppcmd::utility::lexical_cast<decltype(map)::key_type>(it->key()), it->get_##valType());

#define bsoncxx_elem_to_set(elem, set, type) \
    bsoncxx::array::view set##Arr = bsoncxx_get_or_default(elem, array); \
    for (auto it = set##Arr.cbegin(); it != set##Arr.cend(); ++it) \
        set.emplace(it->get_##type());

#define bsoncxx_stream_array_into(array, doc) \
    for (const auto& val : array) \
        doc << val;

#define bsoncxx_stream_map_into(map, doc) \
    for (const auto& [key, val] : map) \
        doc << dppcmd::utility::lexical_cast<std::string>(key) << val;

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};
