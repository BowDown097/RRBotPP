#include "weapons.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/extensions/cache.h"
#include "entities/goods/weapon.h"
#include "utils/ld.h"
#include "utils/random.h"
#include <dpp/dispatcher.h>

Weapons::Weapons() : dppcmd::module<Weapons>("Weapons", "So you've chosen the path of death huh? The rewards here may be pretty large, but be warned - I don't want to hear about any of this \"guilt\" stuff. Proceed with caution.")
{
    register_command(&Weapons::shoot, std::in_place, "shoot", "Blast someone into oblivion with your gun.", "$shoot [user] [gun]");
}

dpp::task<dppcmd::command_result> Weapons::shoot(const dpp::guild_member& member, const dppcmd::remainder<std::string>& gun)
{
    if (member.user_id == context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::BadIdea);
    if (const dpp::user* user = member.get_user(); user->is_bot())
        co_return dppcmd::command_result::from_error(Responses::UserIsBot);

    auto authorMember = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!authorMember)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    const Weapon* weapon = std::ranges::find_if(Constants::Weapons, [&gun](const Weapon& w) {
        return dppcmd::utility::iequals(w.name(), *gun) && w.type() == Weapon::Type::Gun; });
    if (!weapon)
        co_return dppcmd::command_result::from_error(Responses::NotAGun);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    std::string ammo(weapon->ammo());
    if (author.ammo[ammo] <= 0)
        co_return dppcmd::command_result::from_error(Responses::YouNeedAmmo);
    if (!author.weapons.contains(weapon->name()))
        co_return dppcmd::command_result::from_error(std::format(Responses::DontHaveAThing, weapon->name()));

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (target.usingSlots)
        co_return dppcmd::command_result::from_error(std::format(Responses::UserIsGambling, member.get_mention()));

    author.ammo[ammo]--;

    std::string response;
    if (RR::utility::random(100) < weapon->accuracy())
    {
        int damageRoll = RR::utility::random(weapon->damageMin(), weapon->damageMax() + 1);
        target.health -= damageRoll;

        if (target.health > 0)
        {
            response = std::vformat(RR::utility::randomElement(Responses::ShootHits), std::make_format_args(damageRoll));
        }
        else
        {
            std::string cashStr = RR::utility::cash2str(target.cash);
            response = std::vformat(RR::utility::randomElement(Responses::ShootKills), std::make_format_args(cashStr));

            co_await author.setCashWithoutAdjustment(authorMember.value(), author.cash + target.cash, cluster);
            co_await target.setCashWithoutAdjustment(member, 0, cluster);
            target.health = 100;
        }
    }
    else
    {
        response = RR::utility::randomElement(Responses::ShootFails);
    }

    author.modCooldown(author.shootCooldown = Constants::ShootCooldown, authorMember.value());
    MongoManager::updateUser(author);
    MongoManager::updateUser(target);
    co_return dppcmd::command_result::from_success(response);
}
