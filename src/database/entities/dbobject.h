#pragma once
#include <bsoncxx/document/value.hpp>

#define bsoncxx_get_or_default(elem, target) \
    elem ? elem.get_##target() : decltype(elem.get_##target()){}
#define bsoncxx_get_value_or_default(elem, target) \
    elem ? elem.get_##target().value : decltype(elem.get_##target().value){}

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};
