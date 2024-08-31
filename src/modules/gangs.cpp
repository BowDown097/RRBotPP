#include "gangs.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbgang.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dppcmd/extensions/cache.h"
#include "dppcmd/utils/strings.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>
#include <mongocxx/collection.hpp>
#include <regex>

Gangs::Gangs() : dppcmd::module<Gangs>("Gangs", "All about that gang shit.")
{
    register_command(&Gangs::buyVault, "buyvault", "Buy a vault for your gang.");
    register_command(&Gangs::createGang, "creategang", "Create a gang.", "$creategang [name]");
    register_command(&Gangs::deposit, "deposit", "Deposit cash into your gang's vault.", "$deposit [amount]");
    register_command(&Gangs::disband, std::initializer_list<std::string> { "disband", "destroygang" }, "Disband your gang.");
    register_command(&Gangs::gang, std::initializer_list<std::string> { "gang", "ganginfo" }, "View info about your own gang or another gang.", "$gang <name>");
    register_command(&Gangs::invite, "invite", "Invite a user to your gang (if it is private).", "$invite [user]");
    register_command(&Gangs::joinGang, std::initializer_list<std::string> { "join", "joingang" }, "Join a gang.", "$join [name]");
    register_command(&Gangs::kickGangMember, "kickgangmember", "Kick out a member of your gang.", "$kickgangmember [user]");
    register_command(&Gangs::leaveGang, "leavegang", "Leave your current gang.");
    register_command(&Gangs::renameGang, "renamegang", "Rename your gang.", "$renamegang [name]");
    register_command(&Gangs::setPosition, "setposition", "Set a member of your gang's position.", "$setposition [user] [position]");
    register_command(&Gangs::togglePublic, "togglepublic", "Toggles if your gang can be joined without an invite.");
    register_command(&Gangs::transferLeadership, "transferleadership", "Crown a new leader of your gang.", "$transferleadership [user]");
    register_command(&Gangs::vaultBalance, std::initializer_list<std::string> { "vault", "vb", "vaultbal", "vaultbalance" }, "Check how much money is in your gang's vault.");
    register_command(&Gangs::withdrawVault, std::initializer_list<std::string> { "withdrawvault", "wv" }, "Withdraw money from your gang's vault.", "$withdrawvault 10000");
}

dpp::task<dppcmd::command_result> Gangs::buyVault()
{
    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(member->user_id, member->guild_id);
    if (user.gang.empty())
        co_return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, member->guild_id, false);
    if (gang.vaultUnlocked)
        co_return dppcmd::command_result::from_error(Responses::GangHasVault);

    gang.vaultUnlocked = true;
    co_await user.setCashWithoutAdjustment(member.value(), user.cash - Constants::GangVaultCost, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success(std::format(Responses::VaultUnlocked, RR::utility::cash2str(Constants::GangVaultCost)));
}

dpp::task<dppcmd::command_result> Gangs::createGang(const dppcmd::remainder<std::string>& name)
{
    if (name->size() <= 2 || name->size() > 32 || !std::regex_match(*name, std::regex("^[a-zA-Z0-9\x20]*$")))
        co_return dppcmd::command_result::from_error(Responses::InvalidGangName);

    mongocxx::cursor cursor = MongoManager::gangs().find(bsoncxx::builder::stream::document()
        << "guildId" << (int64_t)context->msg.guild_id << bsoncxx::builder::stream::finalize);
    auto nameMatches = [&name](const bsoncxx::document::view& doc) { return dppcmd::utility::iequals(doc["name"].get_string(), *name); };
    if (std::any_of(cursor.begin(), cursor.end(), nameMatches))
        co_return dppcmd::command_result::from_error(Responses::GangAlreadyExists);
    if (std::distance(cursor.begin(), cursor.end()) >= Constants::MaxGangsPerGuild)
        co_return dppcmd::command_result::from_error(std::format(Responses::ReachedMaxGangs, Constants::MaxGangsPerGuild));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(member->user_id, member->guild_id);
    if (!user.gang.empty())
        co_return dppcmd::command_result::from_error(Responses::AlreadyInGang);

    user.gang = *name;
    co_await user.setCashWithoutAdjustment(member.value(), user.cash - Constants::GangCreationCost, cluster);

    DbGang newGang;
    newGang.guildId = context->msg.guild_id;
    newGang.leader = context->msg.author.id;
    newGang.members = {{ context->msg.author.id, 0 }};
    newGang.name = *name;
    MongoManager::gangs().insert_one(newGang.toDocument());

    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success(std::format(Responses::GangCreated,
        *name, RR::utility::cash2str(Constants::GangCreationCost)));
}

dpp::task<dppcmd::command_result> Gangs::deposit(long double amount)
{
    if (amount < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "deposit", RR::utility::cash2str(amount)));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(member->user_id, member->guild_id);
    if (user.gang.empty())
        co_return dppcmd::command_result::from_error(Responses::NotInGang);
    if (user.cash < amount)
        co_return dppcmd::command_result::from_error(std::format(Responses::NotEnoughOfThing, "cash"));

    DbGang gang = MongoManager::fetchGang(user.gang, member->guild_id, false);
    if (!gang.vaultUnlocked)
        co_return dppcmd::command_result::from_error(Responses::GangHasNoVault);

    long double finalAmount = amount / 100.0L * (100.0L - Constants::VaultTaxPercent);
    gang.vaultBalance += finalAmount;
    co_await user.setCashWithoutAdjustment(member.value(), user.cash - finalAmount, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);
    co_return dppcmd::command_result::from_success(std::format(Responses::DepositedIntoVault,
        RR::utility::cash2str(finalAmount), Constants::VaultTaxPercent));
}

dppcmd::command_result Gangs::disband()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::NotGangLeader);

    MongoManager::deleteGang(gang.name, gang.guildId, false);
    user.gang.clear();

    for (const auto& [userId, _] : gang.members)
    {
        if (userId != context->msg.author.id)
        {
            DbUser member = MongoManager::fetchUser(userId, context->msg.guild_id);
            member.gang.clear();
            MongoManager::updateUser(member);
        }
    }

    MongoManager::updateUser(user);
    return dppcmd::command_result::from_success(Responses::DisbandedGang);
}

dppcmd::command_result Gangs::gang(const std::optional<dppcmd::remainder<std::string>>& nameIn)
{
    std::string name;
    if (!nameIn.has_value())
    {
        DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
        if (user.gang.empty())
            return dppcmd::command_result::from_error(Responses::NotInGang);
        name = user.gang;
    }
    else
    {
        name = *nameIn.value();
    }

    DbGang gang = MongoManager::fetchGang(name, context->msg.guild_id, nameIn.has_value());
    if (gang.name.empty())
        return dppcmd::command_result::from_error(Responses::GangNotFound);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(gang.name)
        .add_field("Leader", dpp::user::get_mention(gang.leader));

    for (std::string_view pos : Constants::GangPositions | std::views::drop(1))
    {
        auto posMembers = gang.members
            | std::views::filter([pos](const auto& pair) { return Constants::GangPositions[pair.second] == pos; })
            | std::views::transform([](const auto& pair) { return dpp::user::get_mention(pair.first); });
        embed.add_field(std::string(pos) + 's', dppcmd::utility::join(posMembers, '\n'));
    }

    if (gang.vaultBalance >= 0.01L)
        embed.add_field("Vault Balance", RR::utility::cash2str(gang.vaultBalance));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dppcmd::command_result::from_success();
}

dppcmd::command_result Gangs::invite(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (!target.gang.empty())
        return dppcmd::command_result::from_error(std::format(Responses::UserAlreadyInGang, member.get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.isPublic)
        return dppcmd::command_result::from_error(Responses::InviteGangIsPublic);
    if (gang.members.size() >= Constants::GangMaxMembers)
        return dppcmd::command_result::from_error(std::format(Responses::YourGangAtMaxMembers, Constants::GangMaxMembers));
    if (gang.members[context->msg.author.id] > 1)
        return dppcmd::command_result::from_error(std::format(Responses::NeedHigherGangPosition, Constants::GangPositions[1]));

    target.pendingGangInvites.insert(gang.name);
    MongoManager::updateUser(target);

    return dppcmd::command_result::from_success(std::format(Responses::InvitedUserToGang, member.get_mention()));
}

dppcmd::command_result Gangs::joinGang(const dppcmd::remainder<std::string>& name)
{
    DbGang gang = MongoManager::fetchGang(*name, context->msg.guild_id, true);
    if (gang.name.empty())
        return dppcmd::command_result::from_error(Responses::GangNotFound);
    if (gang.members.size() >= Constants::GangMaxMembers)
        return dppcmd::command_result::from_error(std::format(Responses::GangAtMaxMembers, Constants::GangMaxMembers));

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (!user.gang.empty())
        return dppcmd::command_result::from_error(Responses::AlreadyInGang);
    if (!gang.isPublic && !user.pendingGangInvites.contains(gang.name))
        return dppcmd::command_result::from_error(std::format(Responses::GangIsPrivate, Constants::GangPositions[1]));

    gang.members[context->msg.author.id] = Constants::GangPositions.size() - 1;
    user.gang = gang.name;
    user.pendingGangInvites.erase(gang.name);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    return dppcmd::command_result::from_success(std::format(Responses::JoinedGang, gang.name));
}

dppcmd::command_result Gangs::kickGangMember(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (author.gang != target.gang)
        return dppcmd::command_result::from_error(std::format(Responses::UserNotInYourGang, member.get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.members[context->msg.author.id] > 1)
        return dppcmd::command_result::from_error(std::format(Responses::NeedHigherGangPosition, Constants::GangPositions[1]));
    if (gang.members[context->msg.author.id] > gang.members[member.user_id])
        return dppcmd::command_result::from_error(std::format(Responses::UserHasHigherGangPosition, member.get_mention()));

    gang.members.erase(member.user_id);
    target.gang.clear();

    MongoManager::updateGang(gang);
    MongoManager::updateUser(target);

    return dppcmd::command_result::from_success(std::format(Responses::KickedUserFromGang, member.get_mention()));
}

dppcmd::command_result Gangs::leaveGang()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::NeedTransferGangLeadership);

    gang.members.erase(context->msg.author.id);
    user.gang.clear();

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    return dppcmd::command_result::from_success(Responses::LeftGang);
}

dpp::task<dppcmd::command_result> Gangs::renameGang(const dppcmd::remainder<std::string>& name)
{
    if (name->size() <= 2 || name->size() > 32 || !std::regex_match(*name, std::regex("^[a-zA-Z0-9\x20]*$")))
        co_return dppcmd::command_result::from_error(Responses::InvalidGangName);

    DbGang matchingGang = MongoManager::fetchGang(*name, context->msg.guild_id, true);
    if (!matchingGang.name.empty())
        co_return dppcmd::command_result::from_error(Responses::GangAlreadyExists);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        co_return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        co_return dppcmd::command_result::from_error(Responses::NotGangLeader);

    std::string ogGangName = gang.name;
    gang.name = *name;
    user.gang = *name;

    for (const auto& [userId, _] : gang.members)
    {
        if (userId != context->msg.author.id)
        {
            DbUser member = MongoManager::fetchUser(userId, context->msg.guild_id);
            member.gang = *name;
            MongoManager::updateUser(member);
        }
    }

    MongoManager::updateGang(gang, ogGangName);
    MongoManager::updateUser(user);

    co_return dppcmd::command_result::from_success(std::format(Responses::RenamedGang, *name));
}

dppcmd::command_result Gangs::setPosition(const dpp::guild_member& member, const dppcmd::remainder<std::string>& position)
{
    if (member.user_id == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    auto posMatch = [&position](std::string_view p) { return dppcmd::utility::iequals(*position, p); };
    if (auto it = std::ranges::find_if(Constants::GangPositions, posMatch); it != Constants::GangPositions.end())
    {
        if (*it == Constants::GangPositions.front())
            return dppcmd::command_result::from_error(Responses::SetPositionLeaderCorrection);

        DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
        if (author.gang.empty())
            return dppcmd::command_result::from_error(Responses::NotInGang);

        DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
        if (author.gang != target.gang)
            return dppcmd::command_result::from_error(std::format(Responses::UserNotInYourGang, member.get_mention()));

        DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
        if (gang.leader != context->msg.author.id)
            return dppcmd::command_result::from_error(Responses::NotGangLeader);

        long posDistance = std::distance(Constants::GangPositions.begin(), it);
        if (gang.members[member.user_id] == posDistance)
            return dppcmd::command_result::from_error(std::format(Responses::UserAlreadyHasPosition, member.get_mention(), *it));

        gang.members[member.user_id] = posDistance;
        MongoManager::updateGang(gang);

        return dppcmd::command_result::from_success(std::format(Responses::ChangedUserPosition, member.get_mention(), *it));
    }
    else
    {
        return dppcmd::command_result::from_error(Responses::InvalidGangPosition);
    }
}

dppcmd::command_result Gangs::togglePublic()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::NotGangLeader);

    gang.isPublic = !gang.isPublic;
    MongoManager::updateGang(gang);

    using namespace std::string_view_literals;
    return dppcmd::command_result::from_success(std::format(Responses::GangPublicityToggled, gang.isPublic ? "now"sv : "no longer"sv));
}

dppcmd::command_result Gangs::transferLeadership(const dpp::guild_member& member)
{
    if (member.user_id == context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::BadIdea);
    if (dpp::user* user = member.get_user(); user->is_bot())
        return dppcmd::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(member.user_id, context->msg.guild_id);
    if (author.gang != target.gang)
        return dppcmd::command_result::from_error(std::format(Responses::UserNotInYourGang, member.get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dppcmd::command_result::from_error(Responses::NotGangLeader);

    gang.leader = member.user_id;
    gang.members[context->msg.author.id] = Constants::GangPositions.size() - 1;
    gang.members[member.user_id] = 0;

    MongoManager::updateGang(gang);
    return dppcmd::command_result::from_error(std::format(Responses::TransferredLeadership, member.get_mention()));
}

dppcmd::command_result Gangs::vaultBalance()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (!gang.vaultUnlocked)
        return dppcmd::command_result::from_error(Responses::GangHasNoVault);
    if (gang.vaultBalance < 0.01L)
        return dppcmd::command_result::from_error(Responses::GangIsBroke);

    return dppcmd::command_result::from_success(std::format(Responses::GangVaultBalance, RR::utility::cash2str(gang.vaultBalance)));
}

dpp::task<dppcmd::command_result> Gangs::withdrawVault(long double amount)
{
    if (amount < Constants::TransactionMin)
        co_return dppcmd::command_result::from_error(std::format(Responses::CashInputTooLow, "withdraw", RR::utility::cash2str(Constants::TransactionMin)));

    auto member = dppcmd::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!member)
        co_return dppcmd::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        co_return dppcmd::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (!gang.vaultUnlocked)
        co_return dppcmd::command_result::from_error(Responses::GangHasNoVault);
    if (gang.vaultBalance < amount)
        co_return dppcmd::command_result::from_error(Responses::VaultNotEnoughCash);

    gang.vaultBalance -= amount;
    co_await user.setCashWithoutAdjustment(member.value(), user.cash + amount, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    co_return dppcmd::command_result::from_success(std::format(Responses::WithdrewFromVault, RR::utility::cash2str(amount)));
}
