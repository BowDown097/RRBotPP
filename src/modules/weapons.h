#pragma once
#include "dppcmd/modules/module.h"

namespace dpp { class guild_member; }

class Weapons : public dppcmd::module<Weapons>
{
public:
    Weapons();
private:
    dpp::task<dppcmd::command_result> shoot(const dpp::guild_member& member, const dppcmd::remainder<std::string>& gun);
};
