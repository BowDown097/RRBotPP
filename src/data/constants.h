#pragma once
#include <array>
#include "entities/achievement.h"
#include "entities/goods/ammo.h"
#include "entities/goods/crate.h"
#include "entities/goods/collectible.h"
#include "entities/goods/consumable.h"
#include "entities/goods/perk.h"
#include "entities/goods/tool.h"
#include "entities/goods/weapon.h"

namespace Constants
{
    // ********************
    //     ACHIEVEMENTS
    // ********************
    constexpr std::array DefaultAchievements = {
        Achievement("Hard Boiled Egg", "Find a hard boiled egg.", 1337),
        Achievement("I Just Feel Bad", "Take a million dollar L."),
        Achievement("Jackpot!", "Get a jackpot with $slots."),
        Achievement("Luckiest Dude Alive", "Win $69.69."),
        Achievement("Literally 1984", "Get muted."),
        Achievement("Maxed!", "Reach the max prestige.", 1420),
        Achievement("OH BABY A TRIPLE", "Roll triples in $dice."),
        Achievement("Prestiged!", "Get your first prestige.", 1000),
        Achievement("Pretty Damn Lucky", "Win $99+.")
    };
    constexpr std::array GamblingAchievementNames = {
        "I Just Feel Bad",
        "Jackpot!",
        "Luckiest Dude Alive",
        "OH BABY A TRIPLE",
        "Pretty Damn Lucky"
    };
    // ********************
    //     CASH SYSTEM
    // ********************
    constexpr long double CashMultiplier = 1.0L;
    constexpr long double TransactionMin = 100;
    // ********************
    //      COOLDOWNS
    // ********************
    constexpr long BullyCooldown = 600;
    constexpr long ChopCooldown = 3600;
    constexpr long DailyCooldown = 86400;
    constexpr long DealCooldown = 3600;
    constexpr long DigCooldown = 3600;
    constexpr long FarmCooldown = 3600;
    constexpr long FishCooldown = 3600;
    constexpr long HackCooldown = 3600;
    constexpr long HuntCooldown = 3600;
    constexpr long LootCooldown = 3600;
    constexpr long MessageCashCooldown = 60;
    constexpr long MineCooldown = 3600;
    constexpr long PacifistCooldown = 259200;
    constexpr long PrestigeCooldown = 43200;
    constexpr long RapeCooldown = 3600;
    constexpr long RobCooldown = 3600;
    constexpr long ScavengeCooldown = 60;
    constexpr long ShootCooldown = 14400;
    constexpr long SlaveryCooldown = 3600;
    constexpr long WhoreCooldown = 3600;
    // ********************
    //        CRIME
    // ********************
    constexpr double GenericCrimeWinOdds = 80;
    constexpr long double GenericCrimeLossMax = 420;
    constexpr long double GenericCrimeLossMin = 69;
    constexpr double GenericCrimeToolOdds = 4;
    constexpr long double GenericCrimeWinMax = 690;
    constexpr long double GenericCrimeWinMin = 69;
    constexpr double HackOdds = 10;
    constexpr double RapeOdds = 50;
    constexpr long double RapeMaxPercent = 9;
    constexpr long double RapeMinPercent = 5;
    constexpr double RobOdds = 40;
    constexpr long double RobMaxPercent = 20;
    constexpr long double RobMinCash = 100;
    constexpr long double ScavengeMinCash = 50;
    constexpr long double ScavengeMaxCash = 100;
    constexpr int ScavengeTimeout = 15;
    // ********************
    //       GAMBLING
    // ********************
    constexpr int DoubleOdds = 45;
    constexpr long double PotFee = 3;
    // ********************
    //        GANGS
    // ********************
    constexpr long double GangCreationCost = 5000;
    constexpr int GangMaxMembers = 10;
    constexpr long double GangRenameCost = 2000;
    constexpr long double GangVaultCost = 5000;
    constexpr std::array<std::string_view, 3> GangPositions = { "Leader", "Elder", "Member" };
    constexpr int MaxGangsPerGuild = 50;
    constexpr long double VaultTaxPercent = 1.5L;
    // ********************
    //        GOODS
    // ********************
    constexpr long BlackHatDuration = 3600;
    constexpr long CocaineDuration = 3600;
    constexpr long SkiMaskDuration = 3600;
    constexpr long ViagraDuration = 3600;
    // ********************
    //     INVESTMENTS
    // ********************
    constexpr long double InvestmentFeePercent = 1.5L;
    constexpr long double InvestmentMinAmount = 0.01L;
    // ********************
    //      MODERATION
    // ********************
    constexpr int ChillMaxSeconds = 21600;
    constexpr int ChillMinSeconds = 10;
    // ********************
    //        TASKS
    // ********************
    constexpr std::array<std::pair<const char*, long double>, 3> Fish = {{
        { "carp", 24 },
        { "trout", 27 },
        { "goldfish", 30 }
    }};
    constexpr double FishCoconutOdds = 20;
    constexpr int GenericTaskWoodMax = 65;
    constexpr int GenericTaskWoodMin = 32;
    constexpr int GenericTaskStoneMax = 113;
    constexpr int GenericTaskStoneMin = 65;
    constexpr int GenericTaskIronMax = 161;
    constexpr int GenericTaskIronMin = 113;
    constexpr int GenericTaskDiamondMax = 209;
    constexpr int GenericTaskDiamondMin = 161;
    constexpr int GenericTaskNetheriteMax = 257;
    constexpr int GenericTaskNetheriteMin = 209;
    // ********************
    //        ITEMS
    // ********************
    constexpr std::array Ammos = {
        Ammo("Pistol Round", 3),
        Ammo("Rifle Round", 2),
        Ammo("Sniper Round", 1),
        Ammo("Rocket", 0.5)
    };
    constexpr std::array Crates = {
        Crate("Daily Crate", 0, Crate::Tier::Daily, 1, 0, 1500),
        Crate("Bronze Crate", 5000, Crate::Tier::Bronze, 2),
        Crate("Silver Crate", 10000, Crate::Tier::Silver, 2, 1),
        Crate("Gold Crate", 15000, Crate::Tier::Gold, 4, 2),
        Crate("Diamond Crate", 25000, Crate::Tier::Diamond, 6, 3)
    };
    constexpr std::array Collectibles = {
        Collectible("Ape NFT", 1000, "Who actually likes these? Why does this have value?", "https://i.ibb.co/w0syJ61/nft.png"),
        Collectible("Bank Cheque", -1, "Hey hey hey, we got ourselves some free money!", "https://i.ibb.co/wCYcrP7/Blank-Cheque.png"),
        Collectible("Coconut", 3, "Well this is cool, I guess.", "https://i.ibb.co/svxvLKP/coconut.png"),
        Collectible("V Card", 69696969.69L, "Here you go, ya fuckin' virgin. Get a life bro.", "https://i.ibb.co/rvKXgb5/vcard.png", false)
    };
    constexpr std::array Consumables = {
        Consumable("Black Hat", "Become an epic hax0r.", "You might get busted by the feds and get fined.", "$hack success rate increased by 10%.", BlackHatDuration, 2),
        Consumable("Cocaine", "Snorting a line of this funny sugar makes you HYPER and has some crazy effects.", "You have a chance of overdosing, which will make you lose all your remaining cocaine as well as not be able to use commands with cooldowns for a certain amount of time. The chance of overdosing and how long you can't use economy commands depends on how many lines you have in your system.", "Cooldowns are reduced by 10% for each line snorted.", CocaineDuration),
        Consumable("Ski Mask", "Well, all good robbers wear ski masks right?", "Paying back some bail money might be in your future.", "$rob success rate increased by 10%.", SkiMaskDuration, 2),
        Consumable("Viagra", "Get it goin', if you know what I mean.", "The pill has a chance to backfire and give you ED.", "$rape success rate increased by 10%.", ViagraDuration, 2)
    };
    constexpr std::array Perks = {
        Perk("Enchanter", 5000, "Tasks are 20% more effective, but your tools have a 2% chance of breaking after use.", 172800),
        Perk("Multiperk", 10000, "Grants the ability to equip 2 perks, not including this one.", 604800),
        Perk("Pacifist", 0, "You are immune to all crimes, but you cannot use any crime commands and you also cannot appear on the leaderboard. Cannot be stacked with other perks, even if you have the Multiperk. Can be discarded, but cannot be used again for 3 days.", -1),
        Perk("Speed Demon", 5000, "Cooldowns are 15% shorter, but you have a 5% higher chance of failing any command that can fail.", 172800)
    };
    constexpr std::array Tools = {
        Tool("Wooden Pickaxe", 4500, Tool::Tier::Wood),
        Tool("Stone Pickaxe", 6000, Tool::Tier::Stone, 0, 0, 1.33L),
        Tool("Iron Pickaxe", 7500, Tool::Tier::Iron, 0, 0, 1.66L),
        Tool("Diamond Pickaxe", 9000, Tool::Tier::Diamond, 0, 0, 2),
        Tool("Netherite Pickaxe", 10500, Tool::Tier::Netherite, 0, 0, 2.33L),
        Tool("Wooden Sword", 4500, Tool::Tier::Wood, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Sword", 6000, Tool::Tier::Stone, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Sword", 7500, Tool::Tier::Iron, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Sword", 9000, Tool::Tier::Diamond, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Sword", 10500, Tool::Tier::Netherite, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Shovel", 4500, Tool::Tier::Wood, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Shovel", 6000, Tool::Tier::Stone, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Shovel", 7500, Tool::Tier::Iron, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Shovel", 9000, Tool::Tier::Diamond, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Shovel", 10500, Tool::Tier::Netherite, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Axe", 4500, Tool::Tier::Wood, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Axe", 6000, Tool::Tier::Stone, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Axe", 7500, Tool::Tier::Iron, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Axe", 9000, Tool::Tier::Diamond, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Axe", 10500, Tool::Tier::Netherite, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Wooden Hoe", 4500, Tool::Tier::Wood, GenericTaskWoodMin * 2.5L, GenericTaskWoodMax * 2.5L),
        Tool("Stone Hoe", 6000, Tool::Tier::Stone, GenericTaskStoneMin * 2.5L, GenericTaskStoneMax * 2.5L),
        Tool("Iron Hoe", 7500, Tool::Tier::Iron, GenericTaskIronMin * 2.5L, GenericTaskIronMax * 2.5L),
        Tool("Diamond Hoe", 9000, Tool::Tier::Diamond, GenericTaskDiamondMin * 2.5L, GenericTaskDiamondMax * 2.5L),
        Tool("Netherite Hoe", 10500, Tool::Tier::Netherite, GenericTaskNetheriteMin * 2.5L, GenericTaskNetheriteMax * 2.5L),
        Tool("Fishing Rod", 7500, Tool::Tier::Special, Fish.front().second * 7L, Fish.back().second * 15L)
    };
    constexpr std::array Weapons = {
        Weapon("Glock 17", 30, "Pistol Round", 13, 21, 40, "The classic. Also the weakest. Upgrade when?", "Gun", { "Bronze Crate", "Silver Crate", "Gold Crate", "Diamond Crate" }),
        Weapon("Python", 35, "Pistol Round", 20, 25, 30, "Nice, small, and hits like a truck. Like a Glock but it doesn't suck.", "Gun", { "Silver Crate", "Gold Crate", "Diamond Crate" }),
        Weapon("AR-15", 50, "Rifle Round", 40, 45, 25, "\"Ummmm.. This is like a full on military weapon, we should ban it\" said some fucking loser no one cares about. This gun is awesome.", "Gun", { "Gold Crate", "Diamond Crate" }),
        Weapon("M16", 60, "Rifle Round", 35, 40, 20, "TA-TA-TA! Three round burst. Nice n' accurate. Absolute beauty.", "Gun", { "Gold Crate", "Diamond Crate" }),
        Weapon("Intervention", 70, "Sniper Round", 60, 80, 3, "Big dick energy in a weapon. Sexy. Accurate. Hard-hitting. The bros love it, the hoes love it. I love it.", "Gun", { "Gold Crate", "Diamond Crate" }),
        Weapon("Barrett M82", 60, "Sniper Round", 50, 70, 7, "Like an Intervention but gayer.", "Gun", { "Gold Crate", "Diamond Crate" }),
        Weapon("RPG", 30, "Rocket", 99, 100, 10, "A FUCKIN' ROCKET LAUNCHER!!!! GUN GO BOOM!", "Gun", { "Diamond Crate" })
    };
    // ********************
    //      PRESTIGE
    // ********************
    constexpr int MaxPrestige = 10;
    // ********************
    //      SCAVENGE
    // ********************
    constexpr std::array<std::pair<std::string_view, std::string_view>, 238> ScavengeWordSet = {{
        { "ant", "hormiga" },
        { "zebra", "cebra" },
        { "snail", "caracol" },
        { "boy", "niño" },
        { "zoo", "zoológico" },
        { "bowl", "cuenco" },
        { "purse", "bolso" },
        { "cow", "vaca" },
        { "giraffe", "jirafa" },
        { "crack", "grieta" },
        { "seashell", "concha marina" },
        { "grapes", "uvas" },
        { "hair", "cabello" },
        { "circle", "círculo" },
        { "jellyfish", "medusa" },
        { "desk", "escritorio" },
        { "dinosaur", "dinosaurio" },
        { "ghost", "fantasma" },
        { "branch", "rama" },
        { "pen", "bolígrafo" },
        { "leaf", "hoja" },
        { "flag", "bandera" },
        { "mitten", "manopla" },
        { "horse", "caballo" },
        { "bow", "arco" },
        { "knee", "rodilla" },
        { "whale", "ballena" },
        { "frog", "rana" },
        { "bathroom", "baño" },
        { "pillow", "almohada" },
        { "roly poly", "bicho bolita" },
        { "baby", "bebé" },
        { "caterpillar", "oruga" },
        { "cloud", "nube" },
        { "arm", "brazo" },
        { "man", "hombre" },
        { "pizza", "pizza" },
        { "butterfly", "mariposa" },
        { "sunglasses", "gafas de sol" },
        { "baseball", "béisbol" },
        { "bed", "cama" },
        { "lion", "león" },
        { "ocean", "océano" },
        { "wheel", "rueda" },
        { "bunny", "conejito" },
        { "bat", "murciélago" },
        { "orange", "naranja" },
        { "owl", "búho" },
        { "button", "botón" },
        { "bee", "abeja" },
        { "Earth", "Tierra" },
        { "ladybug", "mariquita" },
        { "triangle", "triángulo" },
        { "shoe", "zapato" },
        { "rain", "lluvia" },
        { "bear", "oso" },
        { "curl", "rizo" },
        { "apple", "manzana" },
        { "comb", "peine" },
        { "milk", "leche" },
        { "bike", "bicicleta" },
        { "crab", "cangrejo" },
        { "bridge", "puente" },
        { "ice cream cone", "cono de helado" },
        { "backpack", "mochila" },
        { "tail", "cola" },
        { "leg", "pierna" },
        { "ears", "orejas" },
        { "diamond", "diamante" },
        { "bird", "pájaro" },
        { "alive", "vivo" },
        { "mountain", "montaña" },
        { "starfish", "estrella de mar" },
        { "feather", "pluma" },
        { "mountains", "montañas" },
        { "egg", "huevo" },
        { "alligator", "caimán" },
        { "hook", "gancho" },
        { "ear", "oreja" },
        { "book", "libro" },
        { "Mickey Mouse", "Mickey Mouse" },
        { "daisy", "margarita" },
        { "drum", "tambor" },
        { "oval", "óvalo" },
        { "turtle", "tortuga" },
        { "cheese", "queso" },
        { "face", "cara" },
        { "eyes", "ojos" },
        { "light", "luz" },
        { "coat", "abrigo" },
        { "candy", "dulce" },
        { "rocket", "cohete" },
        { "moon", "luna" },
        { "pie", "pastel" },
        { "heart", "corazón" },
        { "tree", "árbol" },
        { "cube", "cubo" },
        { "dream", "sueño" },
        { "kitten", "gatito" },
        { "key", "llave" },
        { "football", "fútbol americano" },
        { "snowman", "muñeco de nieve" },
        { "water", "agua" },
        { "river", "río" },
        { "sea", "mar" },
        { "woman", "mujer" },
        { "skateboard", "patineta" },
        { "robot", "robot" },
        { "swimming pool", "piscina" },
        { "rabbit", "conejo" },
        { "clock", "reloj" },
        { "lips", "labios" },
        { "line", "línea" },
        { "pig", "cerdo" },
        { "worm", "gusano" },
        { "duck", "pato" },
        { "plant", "planta" },
        { "hat", "sombrero" },
        { "coin", "moneda" },
        { "fork", "tenedor" },
        { "boat", "barco" },
        { "bread", "pan" },
        { "blocks", "bloques" },
        { "door", "puerta" },
        { "house", "casa" },
        { "crayon", "crayón" },
        { "island", "isla" },
        { "snowflake", "copo de nieve" },
        { "table", "mesa" },
        { "bell", "campana" },
        { "hippo", "hipopótamo" },
        { "camera", "cámara" },
        { "feet", "pies" },
        { "star", "estrella" },
        { "hand", "mano" },
        { "bus", "autobús" },
        { "octopus", "pulpo" },
        { "cherry", "cereza" },
        { "jail", "cárcel" },
        { "dog", "perro" },
        { "banana", "plátano" },
        { "cup", "taza" },
        { "sheep", "oveja" },
        { "fly", "mosca" },
        { "dragon", "dragón" },
        { "cupcake", "magdalena" },
        { "neck", "cuello" },
        { "music", "música" },
        { "bumblebee", "abejorro" },
        { "window", "ventana" },
        { "chimney", "chimenea" },
        { "zigzag", "zigzag" },
        { "jacket", "chaqueta" },
        { "train", "tren" },
        { "bracelet", "pulsera" },
        { "pencil", "lápiz" },
        { "slide", "tobogán" },
        { "doll", "muñeca" },
        { "inchworm", "oruga" },
        { "beak", "pico" },
        { "ants", "hormigas" },
        { "lollipop", "piruleta" },
        { "corn", "maíz" },
        { "chicken", "pollo" },
        { "blanket", "manta" },
        { "socks", "calcetines" },
        { "cat", "gato" },
        { "finger", "dedo" },
        { "nail", "uña" },
        { "airplane", "avión" },
        { "computer", "computadora" },
        { "kite", "cometa" },
        { "family", "familia" },
        { "broom", "escoba" },
        { "motorcycle", "motocicleta" },
        { "snake", "serpiente" },
        { "rainbow", "arcoíris" },
        { "ship", "barco" },
        { "spoon", "cuchara" },
        { "carrot", "zanahoria" },
        { "ball", "pelota" },
        { "bug", "bicho" },
        { "hamburger", "hamburguesa" },
        { "cookie", "galleta" },
        { "mouse", "ratón" },
        { "lamp", "lámpara" },
        { "helicopter", "helicóptero" },
        { "chair", "silla" },
        { "mouth", "boca" },
        { "monster", "monstruo" },
        { "head", "cabeza" },
        { "stairs", "escaleras" },
        { "swing", "columpio" },
        { "smile", "sonrisa" },
        { "fire", "fuego" },
        { "nose", "nariz" },
        { "square", "cuadrado" },
        { "angel", "ángel" },
        { "bunk bed", "litera" },
        { "truck", "camión" },
        { "glasses", "gafas" },
        { "sun", "sol" },
        { "flower", "flor" },
        { "lizard", "lagarto" },
        { "monkey", "mono" },
        { "box", "caja" },
        { "float", "flotar" },
        { "lemon", "limón" },
        { "love", "amor" },
        { "bounce", "rebotar" },
        { "grass", "hierba" },
        { "eye", "ojo" },
        { "person", "persona" },
        { "shirt", "camisa" },
        { "sea turtle", "tortuga marina" },
        { "fish", "pez" },
        { "popsicle", "paleta" },
        { "legs", "piernas" },
        { "night", "noche" },
        { "ring", "anillo" },
        { "spider web", "telaraña" },
        { "basketball", "baloncesto" },
        { "pants", "pantalones" },
        { "suitcase", "maleta" },
        { "jar", "tarro" },
        { "balloon", "globo" },
        { "rock", "roca" },
        { "bench", "banco" },
        { "elephant", "elefante" },
        { "bone", "hueso" },
        { "bark", "ladrido" },
        { "car", "coche" },
        { "spider", "araña" },
        { "girl", "niña" },
        { "candle", "vela" },
        { "beach", "playa" },
        { "king", "rey" }
    }};
}
