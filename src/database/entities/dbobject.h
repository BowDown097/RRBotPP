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

#define bsoncxx_elem_to_map(elem, map, key, keyType, val, valType) \
    bsoncxx::array::view map##Arr = bsoncxx_get_or_default(elem, array); \
    for (auto it = map##Arr.cbegin(); it != map##Arr.cend(); ++it) \
    { \
        bsoncxx::document::view innerDoc = it->get_document(); \
        map.emplace(bsoncxx_get_value_or_default(innerDoc[key], keyType), \
                    bsoncxx_get_value_or_default(innerDoc[val], valType)); \
    }

#define bsoncxx_stream_map_into(map, arr, key, val) \
    for (const auto& [map_key, map_val] : map) \
    { \
        arr << bsoncxx::builder::stream::open_document \
            << key << map_key \
            << val << map_val \
        << bsoncxx::builder::stream::close_document; \
    }

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};
