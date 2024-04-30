#pragma once
#include <bsoncxx/document/value.hpp>

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};
