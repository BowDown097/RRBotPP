#pragma once
#include "database/entities/dbobject.h"
#include <set>

struct DbConfigMisc : DbObject
{
    std::set<std::string> disabledCommands;
    std::set<std::string> disabledModules;
    bool dropsDisabled{};
    std::set<std::string> filteredTerms;
    int64_t guildId{};
    bool inviteFilterEnabled{};
    bool nsfwEnabled{};
    bool scamFilterEnabled{};

    DbConfigMisc() = default;
    explicit DbConfigMisc(bsoncxx::document::view doc);
    bsoncxx::document::value toDocument() const override;
};
