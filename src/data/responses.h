#pragma once
#include <array>
#include <string_view>

namespace Responses
{
// ********************
//       GENERIC
// ********************
constexpr const char* AlreadyHaveAThing = "You already have a(n) {}!";
constexpr const char* AlreadyHaveThing = "You already have {}!";
constexpr const char* BadIdea = "I don't think that's a good idea.";
constexpr const char* CashInputTooLow = "You need to {} at least {}.";
constexpr const char* DontHaveAThing = "You don't have a(n) {}!";
constexpr const char* DontHaveAnyThing = "You don't have any {}(s)!";
constexpr const char* DontHaveThing = "You don't have {}!";
constexpr const char* GetChannelFailed = "Failed to get channel info.";
constexpr const char* GetCryptoValueFailed = "Failed to get crypto value.";
constexpr const char* GetGuildFailed = "Failed to get server info.";
constexpr const char* GetUserFailed = "Failed to get user info.";
constexpr const char* InvalidCurrency = "That is not a currently accepted currency!";
constexpr const char* NonexistentCommand = "You have specified a nonexistent command!";
constexpr const char* NonexistentModule = "You have specified a nonexistent module!";
constexpr const char* NoRanksConfigured = "No ranks are configured.";
constexpr const char* NotEnoughOfThing = "You don't have that much {}!";
constexpr const char* UserAlreadyHasThing = "{} already has a(n) {}!";
constexpr const char* UserHasNotEnoughOfThing = "{} doesn't have that much {}!";
constexpr const char* UserIsBot = "Nope.";
constexpr const char* UserIsBroke = "{} is broke!";
constexpr const char* UserIsGambling = "{} is using the slot machine! Let them finish!";
constexpr const char* YouAreBroke = "You're broke!";
constexpr const char* YouAreGambling = "You appear to be using the slot machine. You can't run any command until it is finished.";
// ********************
//    ADMINISTRATION
// ********************
constexpr const char* GaveUserItem = "Gave {} a(n) **{}**.";
constexpr const char* InvalidPrestigeLevel = "Invalid prestige level!";
constexpr const char* MissingAchievement = "{} doesn't have that achievement!";
constexpr const char* MissingStat = "{} doesn't have that stat!";
constexpr const char* NegativeCash = "You can't set their cash to a negative value!";
constexpr const char* PotDrawing = "Done! The pot should be drawing soon.";
constexpr const char* PotEmpty = "The pot is currently empty.";
constexpr const char* RemovedAchievement = "Removed the achievement from {}.";
constexpr const char* RemovedCrates = "Removed {}'s crates.";
constexpr const char* RemovedStat = "Removed the stat from {}.";
constexpr const char* ResetCooldowns = "Reset {}'s cooldowns.";
constexpr const char* SetCash = "Set {}'s cash to **{}**.";
constexpr const char* SetCrypto = "Set {}'s {} to **{}**.";
constexpr const char* SetPrestige = "Set {}'s prestige level to **{}**.";
constexpr const char* SetStat = "Set {}'s stat for \"{}\" to **{}**.";
// ********************
//      BOT OWNER
// ********************
constexpr const char* NotDisabledCommand = "That is not a disabled command!";
constexpr const char* ResetUser = "Reset {}.";
constexpr const char* SetCommandDisabled = "Disabled the command.";
constexpr const char* SetCommandEnabled = "Enabled the command.";
constexpr const char* SetUserBlacklisted = "Blacklisted {}.";
constexpr const char* SetUserUnblacklisted = "Unblacklisted {}.";
constexpr const char* UserNotBlacklisted = "{} is not blacklisted!";
// ********************
//       CONFIG
// ********************
constexpr const char* AddedRank = "Added {} as a level {} rank that costs **{}**.";
constexpr const char* ChannelNotWhitelisted = "That channel is not in the whitelist!";
constexpr const char* ChannelUnwhitelisted = "Removed {} from the whitelist.";
constexpr const char* ChannelWhitelisted = "Whitelisted {}.";
constexpr const char* ClearedConfig = "All configuration cleared!";
constexpr const char* DisabledFilters = "Disabled filters in {}.";
constexpr const char* FilteredTerm = "Added \"{}\" as a filtered term.";
constexpr const char* InvalidFilteredTerm = "Filtered terms must be alphanumeric (including spaces and hyphens).";
constexpr const char* NoFiltersToDisable = "There are no filters to disable!";
constexpr const char* NotDisabledModule = "That is not a disabled module!";
constexpr const char* SetAdminRole = "Set admin role to {}.";
constexpr const char* SetDjRole = "Set DJ role to {}.";
constexpr const char* SetLogsChannel = "Set logs channel to {}.";
constexpr const char* SetModRole = "Set moderator role to {}.";
constexpr const char* SetModuleDisabled = "Disabled the module.";
constexpr const char* SetModuleEnabled = "Enabled the module.";
constexpr const char* SetPotChannel = "Set pot channel to {}.";
constexpr const char* TermAlreadyFiltered = "That term is already filtered!";
constexpr const char* TermNotFiltered = "That term is not filtered!";
constexpr const char* ToggledInviteFilter = "Toggled invite filter {}.";
constexpr const char* ToggledNsfw = "Toggled age-restricted content {}.";
constexpr const char* ToggledRandomDrops = "Toggled random drops {}.";
constexpr const char* ToggledScamFilter = "Toggled scam filter {}.";
constexpr const char* UnfilteredTerm = "Removed \"{}\" from the filter system.";
// ********************
//        CRIME
// ********************
constexpr const char* Bullied = "You BULLIED {} to **{}**!";
constexpr const char* GotTool = "Well I'll be damned! You also got yourself a(n) {}! Check out ``$module tasks`` to see how you can use it.";
constexpr const char* HackTooLarge = "You can only hack {}% of {}'s {}, that being **{}**.";
constexpr const char* HackTooSmall = "You must hack {} or more.";
constexpr const char* HardBoiledEgg = "This egg is hard boiled, not scrambled. https://cdn.discordapp.com/attachments/661812833771847703/926190266904346725/video0.mov";
constexpr const char* NicknameTooLong = "This nickname is longer than the maximum length (32 characters).";
constexpr const char* RapeFailed = "You got COUNTER-RAPED by {}! YOU paid **{}** in asshole repairs.";
constexpr const char* RapeSuccess = "You DEMOLISHED {}'s asshole! They paid **{}** in asshole repairs.";
constexpr const char* RobTooLarge = "You can only rob {}% of {}'s cash, that being **{}**.";
constexpr const char* RobTooSmall = "There's no point in robbing for less than {}!";
constexpr const char* ScavengeFail = "F and an L, broski. That was not the right answer. It was **{}**.";
constexpr const char* ScavengeScrambleDescription = "**Unscramble this word:**\n{}\n*Type your response in the chat. You have {} seconds!*";
constexpr const char* ScavengeSuccess = "That's right! The answer was **{}**.";
constexpr const char* ScavengeTimeout = "TIMEOUT! You failed to respond within {} seconds. Well, the answer was **{}**.";
constexpr const char* ScavengeTranslateDescription = "**Translate this Spanish word/phrase to English:**\n{}\n*Type your response in the chat. You have {} seconds!*";
constexpr const char* UserHasPacifist = "You cannot {} {} as they have the Pacifist perk on.";
constexpr const char* UserIsStaff = "You cannot {} {} as they are a staff member.";
constexpr std::array<std::string_view, 2> DealFails = {
    "You tripped balls on acid with the boys at a party. After waking up, you realize someone took all the money from your piggy bank, leaving you a whopping **{}** poorer.",
    "The DEA were tipped off about your meth lab and you got caught red handed. You paid **{}** in fines."
};
constexpr std::array<std::string_view, 3> DealSuccesses = {
    "Border patrol let your cocaine-stuffed dog through! You earned **{}** from the cartel.",
    "You continue to capitalize off of some 17 year old's amphetamine addiction, yielding you **{}**.",
    "You sold grass to some elementary schoolers and passed it off as weed. They didn't have a lot of course, only **{}**, but money's money."
};
constexpr std::array<std::string_view, 2> HackFails = {
    "{} actually secured their shit properly, got your info, and sent it off to the feds. You got raided and lost **{} {}** in the process.",
    "Luck was on {}'s side and that hacker dude on Instagram scammed your ass! You only had to pay a 25% down payment, but still sucks. There goes **{} {}**. "
};
constexpr std::array<std::string_view, 2> HackSuccesses = {
    "{} pushed his private keys to GitHub LMFAO! You sniped that shit and got **{} {}**.",
    "You did an ol' SIM swap on {}'s phone and yoinked **{} {}** right off their Coinbase. Easy claps!"
};
constexpr std::array<std::string_view, 2> LootFails = {
    "There happened to be a cop coming out of the donut shop next door. You had to pay **{}** in fines.",
    "The manager gave no fucks and beat the SHIT out of you. You lost **{}** paying for face stitches."
};
constexpr std::array<std::string_view, 3> LootSuccesses = {
    "You joined your local protest, looted a Footlocker, and sold what you got. You earned **{}**.",
    "That mall had a lot of shit! You earned **{}**.",
    "You stole from a gas station because you're a fucking idiot. You earned **{}**, basically nothing."
};
constexpr std::array<std::string_view, 2> RobFails = {
    "You carefully took out {}'s wallet, but they noticed and shanked you when you were on your way out. You lost **{}** of your own in the process.",
    "{} happened to be a cop and threw your ass straight into jail! You lost **{}** of your own in the process."
};
constexpr std::array<std::string_view, 2> RobSuccesses = {
    "You beat the shit out of {} and took **{}** from their ass!",
    "You walked up to {} and yoinked **{}** straight from their pocket, without a trace."
};
constexpr std::array<std::string_view, 2> SlaveryFails = {
    "Some dude died from inhumane working conditions and you had to cobble together **{}** for his family. As if that's gonna do anything for those losers though, lmao.",
    "A slave got away and yoinked **{}** from you. Sad day."
};
constexpr std::array<std::string_view, 3> SlaverySuccesses = {
    "You got loads of 12 year olds to tirelessly mine ender chests on the Oldest Anarchy Server in Minecraft. You made **{}** selling the newfound millions of obsidian to an interested party.",
    "The children working in your labor factory did a good job making shoes in the past hour. You made **{}** from all of them, and lost only like 2 cents paying them their wages.",
    "This cotton is BUSSIN! The Confederacy is proud. You have been awarded **{}**."
};
constexpr std::array<std::string_view, 2> WhoreFails = {
    "You were too ugly and nobody wanted you. You lost **{}** buying clothes for the night.",
    "An undercover cop busted you for prostitution! There goes **{}**."
};
constexpr std::array<std::string_view, 3> WhoreSuccesses = {
    "You went to the club and some weird fat dude sauced you **{}**.",
    "The dude you fucked looked super shady, but he did pay up. You earned **{}**.",
    "You found Chad Thundercock himself! **{}** and some amazing sex. What a great night."
};
// ********************
//       ECONOMY
// ********************
constexpr const char* SaucedUser = "You sauced {} {}.";
constexpr const char* UserBalance = "{} has **{}**.";
constexpr const char* YourBalance = "You have **{}**.";
// ********************
//       EVENTS
// ********************
constexpr const char* BadArgCount = "You must provide {} argument(s)!\nCommand usage: ``{}``";
constexpr const char* BadArgument = "Couldn't understand your input of \"{}\".\n"
                                    "This error info might help: ``{}``\n"
                                    "Or maybe the command usage will: ``{}``";
constexpr const char* BannedFromBot = "You are banned from using the bot!";
constexpr const char* CommandDisabled = "This command is disabled!";
constexpr const char* CommandsDisabled = "Commands are disabled in this channel!";
constexpr const char* ErrorOccurred = "An error occurred while running the command. You should probably report this! Here's some info: ``{}``";
constexpr const char* GuildMemberAmbiguous = "Your user input is ambiguous. Run the command again, but this time with the user being one of these {} results:\n{}";
constexpr const char* JoinMessage = R"(
Thank you for inviting me to your server!
You're gonna want to check out $modules. Use $module to view the commands in each module, and $help to see how to use a command.
The Config module will probably be the most important to look at as an admin or server owner.
There's a LOT to look at, so it's probably gonna take some time to get everything set up, but trust me, it's worth it.
Have fun!
)";
constexpr const char* ModuleDisabled = "The module for this command is disabled!";
// ********************
//         FUN
// ********************
constexpr const char* CatFailed = "404 Cat Not Found :(";
constexpr const char* CatFound = "Found one!";
constexpr const char* DogFailed = "404 Doggy Not Found :(";
constexpr const char* DogFound = "Found one!";
constexpr const char* Gay1 = "Not Gay";
constexpr const char* Gay2 = "Kinda Gay";
constexpr const char* Gay3 = "Gay";
constexpr const char* Gay4 = "Hella Gay!";
constexpr const char* GodWordFailed = "Couldn't get God's word. Is the API dead? Do you perhaps not believe?";
constexpr const char* GodWordTooLong = "You trying to get Him to tell you a Bible 2 or something? This is too long!";
constexpr const char* HeadsImage = "https://i.imgur.com/Y77AMLp.png";
constexpr const char* HeadsTitle = "You flipped... HEADS!";
constexpr const char* MagicConchSays = "The Magic Conch Shell says...";
constexpr const char* Penis1 = "Micropenis LMFAO";
constexpr const char* Penis2 = "Ehhh";
constexpr const char* Penis3 = "Not bad at all!";
constexpr const char* Penis4 = "God damn, he's packin'!";
constexpr const char* SneedImage = "https://static.wikia.nocookie.net/simpsons/images/1/14/Al_Sneed.png/revision/latest?cb=20210430000431";
constexpr const char* TailsImage = "https://i.imgur.com/O3ULvhg.png";
constexpr const char* TailsTitle = "You flipped... TAILS!";
constexpr const char* TerryQuoteFailed = "Couldn't get the quote. Is the API dead?";
constexpr const char* TerryQuoteTooLong = "This quote is longer than 2000 characters, so I can't send it. Sorry.";
constexpr const char* UserIsThisGay = "{} is {}% gay.";
constexpr const char* UserPenis = "{}'s penis: 8{}D";
constexpr const char* WaifuDescription = "Your waifu is **{}**.";
constexpr const char* WaifuTitle = "Say hello to your new waifu!";
constexpr const char* YouAreThisGay = "You are {}% gay.";
constexpr const char* YourPenis = "Your penis: 8{}D";
constexpr std::array MagicConchImages = {
    "https://i.imgur.com/kJmUvt2.png", // i don't think so
    "https://i.imgur.com/h1GvIe6.png", // maybe someday
    "https://i.imgur.com/ozPmMUQ.png", // no
    "https://i.imgur.com/uRynAAx.png", // try asking again
    "https://i.imgur.com/qVjaDSs.png" // yes
};
constexpr std::array<std::pair<std::string_view, std::string_view>, 45> Waifus = {{
    { "Adolf Dripler", "https://i.redd.it/cd9v84v46ma21.jpg" },
    { "Aqua", "https://thicc.mywaifulist.moe/waifus/554/bd320a06a7b1b3b7f44e980a4c8e1ac8a975e575465915f1f13f60efe1108c3f_thumb.jpeg" },
    { "Astolfo", "https://i.pinimg.com/originals/47/0d/3d/470d3d86bfd0502f374b1ae7e4ea73b6.jpg" },
    { "Asuna", "https://i.redd.it/oj81n8bpy4e41.jpg" },
    { "Augustus Caesar, first Roman Emperor", "https://cdn.discordapp.com/attachments/1034079617239224331/1041892312130801784/augustus.jpg" },
    { "Baldi", "https://cdn.shopify.com/s/files/1/0076/4769/0825/products/bb-render-minifigure-baldi-solo-front_1024x1024.png?v=1565975377" },
    { "Barack Obama", "https://upload.wikimedia.org/wikipedia/commons/thumb/8/8d/President_Barack_Obama.jpg/1200px-President_Barack_Obama.jpg" },
    { "carlos", "https://cdn.discordapp.com/attachments/804898294873456701/817271464067072010/unknown.png" },
    { "DaBaby", "https://s3.amazonaws.com/media.thecrimson.com/photos/2021/03/02/205432_1348650.png" },
    { "Drake", "https://cdn.discordapp.com/attachments/804898294873456701/817272071871922226/ee3e7e8c7c26dbef49b8095c1ca90db2.png" },
    { "eduardo", "https://i.imgur.com/1bwSckX.png" },
    { "Emilia", "https://kawaii-mobile.com/wp-content/uploads/2016/10/Re-Zero-Emilia.iPhone-6-Plus-wallpaper-1080x1920.jpg" },
    { "Felix", "https://cdn.discordapp.com/attachments/804898294873456701/817269666845294622/739fa73c-be4f-40c3-a057-50395eb46539.png" },
    { "French", "https://live.staticflickr.com/110/297887549_2dc0ee273f_c.jpg" },
    { "George Lincoln Rockwell", "https://i.ytimg.com/vi/hRlvjkQFQvg/hqdefault.jpg" },
    { "Goku", "https://i1.sndcdn.com/artworks-000558462795-v3asuu-t500x500.jpg" },
    { "Gypsycrusader", "https://i.kym-cdn.com/entries/icons/facebook/000/035/821/cover3.jpg" },
    { "Herbert", "https://upload.wikimedia.org/wikipedia/en/thumb/6/67/Herbert_-_Family_Guy.png/250px-Herbert_-_Family_Guy.png" },
    { "Holo", "https://thicc.mywaifulist.moe/waifus/91/d89a6fa083b95e76b9aa8e3be7a5d5d8dc6ddcb87737d428ffc1b537a0146965_thumb.jpeg" },
    { "juan.", "https://cdn.discordapp.com/attachments/804898294873456701/817275147060772874/unknown.png" },
    { "Keffals", "https://cdn.discordapp.com/attachments/1034079617239224331/1041893493473280000/keffals.png" },
    { "Kizuna Ai", "https://thicc.mywaifulist.moe/waifus/1608/105790f902e38da70c7ac59da446586c86eb19c7a9afc063b974d74b8870c4cc_thumb.png" },
    { "Linus", "https://i.ytimg.com/vi/hAsZCTL__lo/mqdefault.jpg" },
    { "Luke Smith", "https://i.ytimg.com/vi/UWpf4ZSAHBo/maxresdefault.jpg" },
    { "maria", "https://i.imgur.com/4Rj8HRs.png" },
    { "Mental Outlaw", "https://static.wikia.nocookie.net/youtube/images/7/7e/Mental.jpg/revision/latest?cb=20220318072553" },
    { "Midnight", "https://cdn.discordapp.com/attachments/804898294873456701/817268857374375986/653c4c631795ba90acefabb745ba3aa4.png" },
    { "Nagisa", "https://cdn.discordapp.com/attachments/804898294873456701/817270514401280010/3f244bab8ef7beafa5167ef0f7cdfe46.png" },
    { "Oswald Mosley", "https://cdn.britannica.com/16/133916-050-01D4245B/Oswald-Mosley-rally-London.jpg" },
    { "pablo", "https://cdn.discordapp.com/attachments/804898294873456701/817271690391715850/unknown.png" },
    { "Peter Griffin (in 2015)", "https://i.kym-cdn.com/photos/images/original/001/868/400/45d.jpg" },
    { "Pizza Heist Witness from Spiderman 2", "https://cdn.discordapp.com/attachments/804898294873456701/817272392002961438/unknown.png" },
    { "Quagmire", "https://s3.amazonaws.com/rapgenius/1361855949_glenn_quagmire_by_gan187-d3r70hu.png" },
    { "Rem", "https://cdn.discordapp.com/attachments/804898294873456701/817269005526106122/latest.png" },
    { "Rikka", "https://cdn.discordapp.com/attachments/804898294873456701/817269185176141824/db6e77106a10787b339da6e0b590410c.png" },
    { "Rin", "https://thicc.mywaifulist.moe/waifus/106/94da5e87c3dcc9eb3db018b815d067bed46f63f16a7e12357cafa1b530ce1c1a_thumb.jpeg" },
    { "Senjougahara", "https://thicc.mywaifulist.moe/waifus/262/1289a42d80717ce4fb0767ddc6c2a19cae5b897d4efe8260401aaacdba166f6e_thumb.jpeg" },
    { "Shinobu", "https://thicc.mywaifulist.moe/waifus/255/3906aba5167583d163ff90d46f86777242e6ff25550ed8ac915ef04f65a8d041_thumb.jpeg" },
    { "Squidward", "https://upload.wikimedia.org/wikipedia/en/thumb/8/8f/Squidward_Tentacles.svg/1200px-Squidward_Tentacles.svg.png" },
    { "Superjombombo", "https://pbs.twimg.com/profile_images/735305572405366786/LF5j-XcT_400x400.jpg" },
    { "Terry Davis", "https://upload.wikimedia.org/wikipedia/commons/3/34/Terry_A._Davis_2017.jpg" },
    { "Warren G. Harding, 29th President of the United States", "https://assets.atlasobscura.com/article_images/18223/image.jpg" },
    { "Your mom (ew bro that's weird)", "https://s.abcnews.com/images/Technology/whale-gty-jt-191219_hpMain_16x9_1600.jpg" },
    { "Zero Two", "https://cdn.discordapp.com/attachments/804898294873456701/817269546024042547/c4c54c906261b82f9401b60daf0e5be2.png" },
    { "Zimbabwe", "https://cdn.discordapp.com/attachments/802654650040844380/817273008821108736/unknown.png" }
}};
// ********************
//       GAMBLING
// ********************
constexpr const char* AddedIntoPot = "Added **{}** into the pot.";
constexpr const char* BetDescription = "{} is betting **{}** against {}.\n"
                                       "Their number is **{}**.\n"
                                       "{}, if you want to accept this bet, respond with a number between 1 and 100 in the next 30 seconds.";
constexpr const char* BetResult = "{} was the closest to my number, **{}**! They're taking home a fine **{}**.";
constexpr const char* Dice1Match = "Good stuff! 1 match. You got paid out your bet.";
constexpr const char* Dice2Matches = "DOUBLES! Now we're cooking with gas. You got paid out DOUBLE your bet (**{}**).";
constexpr const char* Dice3Matches = "WOOOAAHHH! Good shit, man! That's a fine set of TRIPLES you just rolled. You got paid out TEN TIMES your bet (**{}**).";
constexpr const char* DiceNoMatches = "Well damn! There were no matches with your number. Sucks to be you, because you lost **{}**.";
constexpr const char* DiceTitle = "Let's see your roll...";
constexpr const char* Doubled = "I have doubled your cash.";
constexpr const char* GenericGambleFail = "Well damn, you rolled a {}, which wasn't enough. You lost **{}**.\nBalance: {}";
constexpr const char* GenericGambleSuccess = "Good shit my guy! You rolled a {:.2f} and got yourself **{}**!\nBalance: {}";
constexpr const char* InputNotInRange = "Your number needs to be between {} and {}.";
constexpr const char* PotDrawn = "The pot has been drawn, and our LUCKY WINNER is {}!!! After a fee of {:.1f}%, they have won {} with a {:.1f}% chance of winning the pot!";
constexpr const char* UserDidntRespond = "{} didn't respond in time.";
constexpr const char* UserPickedSameNumber = "This fucking goober, {}, responded with the same number. Now you have to do this all over again.";
// ********************
//        GANGS
// ********************
constexpr const char* AlreadyInGang = "You are already in a gang!";
constexpr const char* ChangedUserPosition = "Changed {} to a(n) **{}**.";
constexpr const char* DepositedIntoVault = "Deposited **{}** into your gang's vault (after {}% tax).";
constexpr const char* DisbandedGang = "Your gang has been disbanded!";
constexpr const char* GangAlreadyExists = "There is already a gang with that name.";
constexpr const char* GangAtMaxMembers = "That gang has already reached the maximum of {} members.";
constexpr const char* GangCreated = "Created a gang with the name **{}** for {}.";
constexpr const char* GangHasNoVault = "Your gang does not have a vault!";
constexpr const char* GangHasVault = "Your gang already has a vault!";
constexpr const char* GangIsBroke = "Your gang is broke!";
constexpr const char* GangIsPrivate = "That gang is private! You will need to be invited by a(n) {} or above.";
constexpr const char* GangNotFound = "There is no gang with that name.";
constexpr const char* GangPublicityToggled = "Your gang is {} public.";
constexpr const char* GangVaultBalance = "Your gang's vault has {}.";
constexpr const char* InvalidGangName = "Gang names must be alphanumeric (including spaces) and between 3-32 characters.";
constexpr const char* InvalidGangPosition = "That is not a valid gang position!";
constexpr const char* InviteGangIsPublic = "No need to invite people! Your gang is public!";
constexpr const char* InvitedUserToGang = "Invited {} to your gang.";
constexpr const char* JoinedGang = "You are now a member of **{}**!";
constexpr const char* KickedUserFromGang = "Kicked {} from your gang.";
constexpr const char* LeftGang = "You left your gang.";
constexpr const char* NeedHigherGangPosition = "You need to be a(n) {} or higher in your gang.";
constexpr const char* NeedTransferGangLeadership = "You'll need to transfer leadership first.";
constexpr const char* NotGangLeader = "You are not the leader of your gang!";
constexpr const char* NotInGang = "You are not in a gang!";
constexpr const char* ReachedMaxGangs = "This server has already reached the maximum of {} gangs.";
constexpr const char* RenamedGang = "Renamed your gang to **{}**!";
constexpr const char* SetPositionLeaderCorrection = "Use $transferleadership.";
constexpr const char* TransferredLeadership = "Transferred leadership to {}.";
constexpr const char* UserAlreadyHasPosition = "{} is already a(n) {}!";
constexpr const char* UserAlreadyInGang = "{} is already in a gang!";
constexpr const char* UserHasHigherGangPosition = "{} is in a higher gang position than you!";
constexpr const char* UserNotInYourGang = "{} is not in your gang!";
constexpr const char* VaultNotEnoughCash = "Your gang's vault doesn't have that much money!";
constexpr const char* VaultUnlocked = "Unlocked a vault for your gang for **{}**!";
constexpr const char* WithdrewFromVault = "Withdrew **{}** from your gang's vault.";
constexpr const char* YourGangAtMaxMembers = "Your gang has already reached the maximum of {} members.";
// ********************
//       GENERAL
// ********************
constexpr const char* HelpGenericResponse = "Use $modules to see all of this bot's modules and use $module to view the commands in a module.";
constexpr const char* InfoDescription = "Testing bot";
constexpr const char* InfoFooter = "Developer: BowDown097 • Please contribute! You will be added to this list.";
constexpr const char* InfoTitle = "Rush Reborn Bot TESTING";
constexpr const char* InfoSupportDiscord = "[Join](https://discord.gg/USpJnaaNap)";
constexpr const char* UserHasNoAchs = "{} has no achievements!";
constexpr const char* UserHasNoStats = "{} has no available stats!";
constexpr const char* YouHaveNoAchs = "You have no achievements!";
constexpr const char* YouHaveNoStats = "You have no available stats!";
// ********************
//        GOODS
// ********************
constexpr const char* BlackHatFail = "Dammit! The feds caught onto you! You were fined **{}**.";
constexpr const char* BlackHatSuccess = "Oh yeah. Hacker mode activated. 10% greater $hack chance.";
constexpr const char* BoughtCrate = "You got yourself a {} for **{}**!";
constexpr const char* BoughtPacifistRecently = "You bought the Pacifist perk less than 3 days ago. You still have to wait {}.";
constexpr const char* BoughtPacifistPerk = "You got yourself the Pacifist perk for **{}**! Additionally, as you bought the Pacifist perk, any perks you previously had have been refunded.";
constexpr const char* BoughtPerk = "You got yourself the {} perk for **{}**!";
constexpr const char* BoughtTool = "You got yourself a fresh {} for **{}**!";
constexpr const char* CanOnlyDiscardPacifist = "No perks other than Pacifist can be discarded!";
constexpr const char* CantBeDiscarded = "This item cannot be discarded!";
constexpr const char* CantBePurchased = "This item cannot be purchased!";
constexpr const char* CocaineFail = "OH SHIT, HOMIE! You overdosed! This is why you don't do drugs! You lost all your remaining cocaine and have to go into recovery for {} hour(s), meaning no economy commands for you!";
constexpr const char* CocaineSuccess = "PHEW WEE! That nose candy is already making you feel pumped as FUCK! Your cooldowns have been reduced by a solid 10%.";
constexpr const char* CollectibleFoundDescription = "**{}**: {}\n\nWorth {} - $discard this item to cash in!";
constexpr const char* CollectibleFoundTitle = "Collectible found!";
constexpr const char* DiscardedPacifist = "You discarded your Pacifist perk. If you wish to buy it again, you will have to wait 3 days.";
constexpr const char* GotDailyCrate = "Here's a Daily crate, my good man! Best of luck.";
constexpr const char* HavePacifistPerk = "You have the Pacifist perk and cannot buy another.";
constexpr const char* InCratesOnly = "That item can only be obtained from crates!";
constexpr const char* ItemDiscarded = "You gave your {} to some dude for **{}**.";
constexpr const char* NotAConsumable = "That is not a consumable!";
constexpr const char* NotACrate = "That is not a crate!";
constexpr const char* NotAGun = "That is not a gun!";
constexpr const char* NotAnItem = "That is not an item!";
constexpr const char* ReachedMaxCrates = "You already reached the maximum amount of {}s (10).";
constexpr const char* SkiMaskFail = "You got arrested on suspicion and charged for some other shit before you could even do anything! Systemic racism in action. A homie bailed you out for **{}**, and you had to pay him back.";
constexpr const char* SkiMaskSuccess = "Damn, you look good in this! You're a natural! You've now got a 10% higher success rate when robbing people.";
constexpr const char* UsedMaxConsumable = "You cannot use more than {} {}(s)!";
constexpr const char* ViagraFail = "The pill was laced and your dick stopped working completely! You had to pay **{}** to get that sorted out.";
constexpr const char* ViagraSuccess = "Zoo wee mama! You can feel your blood rushing straight to your dong. You're now 10% more likely for a rape to land.";
constexpr const char* UserHasNothing = "{} has nothing!";
constexpr const char* YouHaveNothing = "You've got nothing!";
// ********************
//     INVESTMENTS
// ********************
constexpr const char* InvestmentSuccess = "You invested in **{} {}**, currently valued at **{}**.";
constexpr const char* InvestmentTooLow = "The cash amount converts to less than {} of {}, which is too low.\nYou'll need to invest at least **{}**.";
constexpr const char* WithdrawSuccess = "You withdrew **{} {}**, currently valued at **{}**.\nA {}% withdrawal fee was taken from this amount, leaving you **{}** richer.";
// ********************
//      MODERATION
// ********************
constexpr const char* ActionFailed = "{} failed: {}.";
constexpr const char* BannedUser = "Banned {}"; // do not add period!
constexpr const char* CantDoActionOnStaff = "You cannot {} {} because they are a staff member.";
constexpr const char* ChannelAlreadyChilled = "This channel is already chilled.";
constexpr const char* ChannelChilled = "Chilled the chat for {}.";
constexpr const char* ChannelNotChilled = "This channel is not chilled.";
constexpr const char* ChannelThawed = "This channel has thawed out! Continue the chaos!";
constexpr const char* InvalidChillDuration = "You cannot chill the chat for {} than {}!";
constexpr const char* InvalidDuration = "You have specified an invalid amount of time!";
constexpr const char* KickedUser = "Kicked {}"; // do not add period!
constexpr const char* MutedUser = "Muted {} for {}"; // do not add period!
constexpr const char* MuteTooLong = "You cannot mute for longer than 28 days!";
constexpr const char* PurgeFoundNoMessages = "There are no messages to delete matching your input.";
constexpr const char* PurgeRangeDidNotReachEnd = "WARNING: The end of your range was further from the beginning message than could be purged. You will have to run this command again.";
constexpr const char* PurgeSuccess = "Purged {} messages.";
constexpr const char* PurgeTooManyMessages = "You can only purge up to 100 messages at once!";
constexpr const char* UnbanFailed = "There is no banned user with that ID!";
constexpr const char* UnmutedUser = "Unmuted {}.";
constexpr const char* UserAlreadyMuted = "{} is already muted!";
constexpr const char* UserNotMuted = "{} is not muted!";
// ********************
//       PRESTIGE
// ********************
constexpr const char* MaxPrestigeReached = "You have already reached the maximum prestige of {}.";
constexpr const char* NoPrestige = "You haven't prestiged yet!";
constexpr const char* PrestigeCancelled = "Prestige cancelled.";
constexpr const char* PrestigePrompt = "Are you SURE you want to prestige? If you don't know already,\nyou will **GET**:\n- +1.2x cash multiplier\n- +1.5x rank costs\n- A shiny, cool new badge on $prestigeinfo\n\nand you will **LOSE**:\n- All money, including in crypto investments\n- All cooldowns\n- All items\n**Respond with \"YES\" if you are sure that you want to prestige. You have {} seconds.**";
constexpr const char* PrestigeSuccess = "Congratulations, homie! You're now Prestige {}. Check $prestigeinfo for your new prestige perks. Hope you said your goodbyes to all of your stuff, cause it's gone!";
// ********************
//        TASKS
// ********************
constexpr const char* TaskComplete = "You {} {} {} with your {} and earned **{}**.\nBalance: {}";
constexpr const char* ToolBroke = "Your {} broke into pieces as soon as you tried to use it. You made no money.";
// ********************
//       WEAPONS
// ********************
constexpr std::array<std::string_view, 3> ShootFails = {
    "The dude did some fucking Matrix shit and literally dodged your bullet. What in the sac of nuts.",
    "The gun jammed and the son of a bitch got away. Looks like it's time to kill the retard that made this gun too.",
    "You just straight up missed! Skill issue."
};
constexpr std::array<std::string_view, 2> ShootHits = {
    "Clean shot. Right in the leg. Cool beans. You dealt **{}** damage.",
    "You saw a miracle today bro! You shot the dude in the FACE and he lived. He certainly don't look like a miracle now though lmao. You dealt **{}** damage."
};
constexpr std::array<std::string_view, 2> ShootKills = {
    "​HIS HEAD FUCKING BLEW UP LMFAO 😂! GUTS AND GORE BABY! You got **{}**.",
    "Well damn, that was nice and easy. Dude went down like nothing and is now in a river somewhere 100 miles away. You got **{}**."
};
constexpr const char* YouNeedAmmo = "You need ammo!";
}
