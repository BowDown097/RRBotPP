#include "gangs.h"
#include "data/constants.h"
#include "data/responses.h"
#include "database/entities/dbgang.h"
#include "database/entities/dbuser.h"
#include "database/mongomanager.h"
#include "dpp-command-handler/extensions/cache.h"
#include "dpp-command-handler/utils/strings.h"
#include "utils/ld.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <format>
#include <mongocxx/collection.hpp>
#include <regex>

Gangs::Gangs() : dpp::module_base("Gangs", "All about that gang shit.")
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

dpp::task<dpp::command_result> Gangs::buyVault()
{
    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(gm->user_id, gm->guild_id);
    if (user.gang.empty())
        co_return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, gm->guild_id, false);
    if (gang.vaultUnlocked)
        co_return dpp::command_result::from_error(Responses::GangHasVault);

    gang.vaultUnlocked = true;
    co_await user.setCashWithoutAdjustment(gm.value(), user.cash - Constants::GangVaultCost, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::VaultUnlocked, RR::utility::curr2str(Constants::GangVaultCost)));
}

dpp::task<dpp::command_result> Gangs::createGang(const dpp::remainder<std::string>& name)
{
    if (name->size() <= 2 || name->size() > 32 || !std::regex_match(*name, std::regex("^[a-zA-Z0-9\x20]*$")))
        co_return dpp::command_result::from_error(Responses::InvalidGangName);

    mongocxx::cursor cursor = MongoManager::gangs().find(bsoncxx::builder::stream::document()
        << "guildId" << (int64_t)context->msg.guild_id << bsoncxx::builder::stream::finalize);
    if (std::any_of(cursor.begin(), cursor.end(), [&name](const bsoncxx::document::view& doc) { return dpp::utility::iequals(doc["name"].get_string(), *name); }))
        co_return dpp::command_result::from_error(Responses::GangAlreadyExists);
    if (std::distance(cursor.begin(), cursor.end()) >= Constants::MaxGangsPerGuild)
        co_return dpp::command_result::from_error(std::format(Responses::ReachedMaxGangs, Constants::MaxGangsPerGuild));

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(gm->user_id, gm->guild_id);
    if (!user.gang.empty())
        co_return dpp::command_result::from_error(Responses::AlreadyInGang);

    user.gang = *name;
    co_await user.setCashWithoutAdjustment(gm.value(), user.cash - Constants::GangCreationCost, cluster);

    DbGang newGang;
    newGang.guildId = context->msg.guild_id;
    newGang.leader = context->msg.author.id;
    newGang.members = {{ context->msg.author.id, 0 }};
    newGang.name = *name;
    MongoManager::gangs().insert_one(newGang.toDocument());

    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::GangCreated,
        *name, RR::utility::curr2str(Constants::GangCreationCost)));
}

dpp::task<dpp::command_result> Gangs::deposit(const cash_in& amountIn)
{
    long double amount = amountIn.top_result();
    if (amount < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::DepositTooLow, RR::utility::curr2str(amount)));

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(gm->user_id, gm->guild_id);
    if (user.gang.empty())
        co_return dpp::command_result::from_error(Responses::NotInGang);
    if (user.cash < amount)
        co_return dpp::command_result::from_error(Responses::NotEnoughCash);

    DbGang gang = MongoManager::fetchGang(user.gang, gm->guild_id, false);
    if (!gang.vaultUnlocked)
        co_return dpp::command_result::from_error(Responses::GangHasNoVault);

    long double finalAmount = amount / 100.0L * (100.0L - Constants::VaultTaxPercent);
    gang.vaultBalance += finalAmount;
    co_await user.setCashWithoutAdjustment(gm.value(), user.cash - finalAmount, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);
    co_return dpp::command_result::from_success(std::format(Responses::DepositedIntoVault,
        RR::utility::curr2str(finalAmount), Constants::VaultTaxPercent));
}

dpp::command_result Gangs::disband()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dpp::command_result::from_error(Responses::NotGangLeader);

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
    return dpp::command_result::from_success(Responses::DisbandedGang);
}

dpp::command_result Gangs::gang(const std::optional<dpp::remainder<std::string>>& nameIn)
{
    std::string name;
    if (!nameIn.has_value())
    {
        DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
        if (user.gang.empty())
            return dpp::command_result::from_error(Responses::NotInGang);
        name = user.gang;
    }
    else
    {
        name = *nameIn.value();
    }

    DbGang gang = MongoManager::fetchGang(name, context->msg.guild_id, nameIn.has_value());
    if (gang.name.empty())
        return dpp::command_result::from_error(Responses::GangNotFound);

    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::red)
        .set_title(gang.name)
        .add_field("Leader", dpp::user::get_mention(gang.leader));

    for (std::string_view pos : Constants::GangPositions | std::views::drop(1))
    {
        auto posMembers = gang.members
            | std::views::filter([pos](const auto& pair) { return Constants::GangPositions[pair.second] == pos; })
            | std::views::transform([](const auto& pair) { return dpp::user::get_mention(pair.first); });
        embed.add_field(std::string(pos) + 's', dpp::utility::join(posMembers, '\n'));
    }

    if (gang.vaultBalance >= 0.01L)
        embed.add_field("Vault Balance", RR::utility::curr2str(gang.vaultBalance));

    context->reply(dpp::message(context->msg.channel_id, embed));
    return dpp::command_result::from_success();
}

dpp::command_result Gangs::invite(const dpp::user_in& userIn)
{
    dpp::user* user = userIn.top_result();
    if (user->id == context->msg.author.id)
        return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (!target.gang.empty())
        return dpp::command_result::from_error(std::format(Responses::UserAlreadyInGang, user->get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.isPublic)
        return dpp::command_result::from_error(Responses::InviteGangIsPublic);
    if (gang.members.size() >= Constants::GangMaxMembers)
        return dpp::command_result::from_error(std::format(Responses::YourGangAtMaxMembers, Constants::GangMaxMembers));
    if (gang.members[context->msg.author.id] > 1)
        return dpp::command_result::from_error(std::format(Responses::NeedHigherGangPosition, Constants::GangPositions[1]));

    target.pendingGangInvites.push_back(gang.name);
    MongoManager::updateUser(target);

    return dpp::command_result::from_success(std::format(Responses::InvitedUserToGang, user->get_mention()));
}

dpp::command_result Gangs::joinGang(const dpp::remainder<std::string>& name)
{
    DbGang gang = MongoManager::fetchGang(*name, context->msg.guild_id, true);
    if (gang.name.empty())
        return dpp::command_result::from_error(Responses::GangNotFound);
    if (gang.members.size() >= Constants::GangMaxMembers)
        return dpp::command_result::from_error(std::format(Responses::GangAtMaxMembers, Constants::GangMaxMembers));

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (!user.gang.empty())
        return dpp::command_result::from_error(Responses::AlreadyInGang);
    if (!gang.isPublic && !std::ranges::contains(user.pendingGangInvites, gang.name))
        return dpp::command_result::from_error(std::format(Responses::GangIsPrivate, Constants::GangPositions[1]));

    gang.members[context->msg.author.id] = Constants::GangPositions.size() - 1;
    user.gang = gang.name;
    std::erase(user.pendingGangInvites, gang.name);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    return dpp::command_result::from_success(std::format(Responses::JoinedGang, gang.name));
}

dpp::command_result Gangs::kickGangMember(const dpp::user_in& userIn)
{
    dpp::user* user = userIn.top_result();
    if (user->id == context->msg.author.id)
        return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (author.gang != target.gang)
        return dpp::command_result::from_error(std::format(Responses::UserNotInYourGang, user->get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.members[context->msg.author.id] > 1)
        return dpp::command_result::from_error(std::format(Responses::NeedHigherGangPosition, Constants::GangPositions[1]));
    if (gang.members[context->msg.author.id] > gang.members[user->id])
        return dpp::command_result::from_error(std::format(Responses::UserHasHigherGangPosition, user->get_mention()));

    gang.members.erase(user->id);
    target.gang.clear();

    MongoManager::updateGang(gang);
    MongoManager::updateUser(target);

    return dpp::command_result::from_success(std::format(Responses::KickedUserFromGang, user->get_mention()));
}

dpp::command_result Gangs::leaveGang()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader == context->msg.author.id)
        return dpp::command_result::from_error(Responses::NeedTransferGangLeadership);

    gang.members.erase(context->msg.author.id);
    user.gang.clear();

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    return dpp::command_result::from_success(Responses::LeftGang);
}

dpp::task<dpp::command_result> Gangs::renameGang(const dpp::remainder<std::string>& name)
{
    if (name->size() <= 2 || name->size() > 32 || !std::regex_match(*name, std::regex("^[a-zA-Z0-9\x20]*$")))
        co_return dpp::command_result::from_error(Responses::InvalidGangName);

    DbGang matchingGang = MongoManager::fetchGang(*name, context->msg.guild_id, true);
    if (!matchingGang.name.empty())
        co_return dpp::command_result::from_error(Responses::GangAlreadyExists);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        co_return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        co_return dpp::command_result::from_error(Responses::NotGangLeader);

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

    co_return dpp::command_result::from_success(std::format(Responses::RenamedGang, *name));
}

dpp::command_result Gangs::setPosition(const dpp::user_in& userIn, const dpp::remainder<std::string>& position)
{
    dpp::user* user = userIn.top_result();
    if (user->id == context->msg.author.id)
        return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    auto posMatch = [&position](std::string_view p) { return dpp::utility::iequals(*position, p); };
    if (auto it = std::ranges::find_if(Constants::GangPositions, posMatch); it != Constants::GangPositions.end())
    {
        if (*it == Constants::GangPositions.front())
            return dpp::command_result::from_error(Responses::SetPositionLeaderCorrection);

        DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
        if (author.gang.empty())
            return dpp::command_result::from_error(Responses::NotInGang);

        DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
        if (author.gang != target.gang)
            return dpp::command_result::from_error(std::format(Responses::UserNotInYourGang, user->get_mention()));

        DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
        if (gang.leader != context->msg.author.id)
            return dpp::command_result::from_error(Responses::NotGangLeader);

        long posDistance = std::distance(Constants::GangPositions.begin(), it);
        if (gang.members[user->id] == posDistance)
            return dpp::command_result::from_error(std::format(Responses::UserAlreadyHasPosition, user->get_mention(), *it));

        gang.members[user->id] = posDistance;
        MongoManager::updateGang(gang);

        return dpp::command_result::from_success(std::format(Responses::ChangedUserPosition, user->get_mention(), *it));
    }
    else
    {
        return dpp::command_result::from_error(Responses::InvalidGangPosition);
    }
}

dpp::command_result Gangs::togglePublic()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dpp::command_result::from_error(Responses::NotGangLeader);

    gang.isPublic = !gang.isPublic;
    MongoManager::updateGang(gang);

    using namespace std::string_view_literals;
    return dpp::command_result::from_success(std::format(Responses::GangPublicityToggled, gang.isPublic ? "now"sv : "no longer"sv));
}

dpp::command_result Gangs::transferLeadership(const dpp::user_in& userIn)
{
    dpp::user* user = userIn.top_result();
    if (user->id == context->msg.author.id)
        return dpp::command_result::from_error(Responses::BadIdea);
    if (user->is_bot())
        return dpp::command_result::from_error(Responses::UserIsBot);

    DbUser author = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (author.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbUser target = MongoManager::fetchUser(user->id, context->msg.guild_id);
    if (author.gang != target.gang)
        return dpp::command_result::from_error(std::format(Responses::UserNotInYourGang, user->get_mention()));

    DbGang gang = MongoManager::fetchGang(author.gang, context->msg.guild_id, false);
    if (gang.leader != context->msg.author.id)
        return dpp::command_result::from_error(Responses::NotGangLeader);

    gang.leader = user->id;
    gang.members[context->msg.author.id] = Constants::GangPositions.size() - 1;
    gang.members[user->id] = 0;

    MongoManager::updateGang(gang);
    return dpp::command_result::from_error(std::format(Responses::TransferredLeadership, user->get_mention()));
}

dpp::command_result Gangs::vaultBalance()
{
    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (!gang.vaultUnlocked)
        return dpp::command_result::from_error(Responses::GangHasNoVault);
    if (gang.vaultBalance < 0.01L)
        return dpp::command_result::from_error(Responses::GangIsBroke);

    return dpp::command_result::from_success(std::format(Responses::GangVaultBalance, RR::utility::curr2str(gang.vaultBalance)));
}

dpp::task<dpp::command_result> Gangs::withdrawVault(const cash_in& amountIn)
{
    long double amount = amountIn.top_result();
    if (amount < Constants::TransactionMin)
        co_return dpp::command_result::from_error(std::format(Responses::WithdrawTooLow, RR::utility::curr2str(Constants::TransactionMin)));

    std::optional<dpp::guild_member> gm = dpp::find_guild_member_opt(context->msg.guild_id, context->msg.author.id);
    if (!gm)
        co_return dpp::command_result::from_error(Responses::GetUserFailed);

    DbUser user = MongoManager::fetchUser(context->msg.author.id, context->msg.guild_id);
    if (user.gang.empty())
        co_return dpp::command_result::from_error(Responses::NotInGang);

    DbGang gang = MongoManager::fetchGang(user.gang, context->msg.guild_id, false);
    if (!gang.vaultUnlocked)
        co_return dpp::command_result::from_error(Responses::GangHasNoVault);
    if (gang.vaultBalance < amount)
        co_return dpp::command_result::from_error(Responses::VaultNotEnoughCash);

    gang.vaultBalance -= amount;
    co_await user.setCashWithoutAdjustment(gm.value(), user.cash + amount, cluster);

    MongoManager::updateGang(gang);
    MongoManager::updateUser(user);

    co_return dpp::command_result::from_success(std::format(Responses::WithdrewFromVault, RR::utility::curr2str(amount)));
}
