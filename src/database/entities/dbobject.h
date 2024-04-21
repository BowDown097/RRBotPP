#ifndef DBOBJECT_H
#define DBOBJECT_H
#include <bsoncxx/document/value.hpp>

struct DbObject
{
    virtual bsoncxx::document::value toDocument() const = 0;
};

#endif  // DBOBJECT_H
