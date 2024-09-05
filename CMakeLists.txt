cmake_minimum_required(VERSION 3.5)
project(RRBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_COROUTINES "" ON)
add_subdirectory(dppcmd)
add_subdirectory(dppinteract)

add_executable(RRBot
    src/events.h src/events.cpp
    src/main.cpp
    src/data/constants.h
    src/data/credentials.h src/data/credentials.cpp
    src/data/responses.h
    src/database/mongomanager.h src/database/mongomanager.cpp
    src/database/entities/dbban.h src/database/entities/dbban.cpp
    src/database/entities/dbchill.h src/database/entities/dbchill.cpp
    src/database/entities/dbgang.h src/database/entities/dbgang.cpp
    src/database/entities/dbobject.h
    src/database/entities/dbpot.h src/database/entities/dbpot.cpp
    src/database/entities/dbuser.h src/database/entities/dbuser.cpp
    src/database/entities/config/dbconfigchannels.h src/database/entities/config/dbconfigchannels.cpp
    src/database/entities/config/dbconfigglobal.h src/database/entities/config/dbconfigglobal.cpp
    src/database/entities/config/dbconfigmisc.h src/database/entities/config/dbconfigmisc.cpp
    src/database/entities/config/dbconfigranks.h src/database/entities/config/dbconfigranks.cpp
    src/database/entities/config/dbconfigroles.h src/database/entities/config/dbconfigroles.cpp
    src/entities/achievement.h
    src/entities/embed.h src/entities/embed.cpp
    src/entities/sizedcache.h src/entities/sizedcache.cpp
    src/entities/goods/ammo.h
    src/entities/goods/collectible.h
    src/entities/goods/consumable.h
    src/entities/goods/crate.h src/entities/goods/crate.cpp
    src/entities/goods/item.h
    src/entities/goods/perk.h
    src/entities/goods/tool.h
    src/entities/goods/weapon.h
    src/modules/administration.h src/modules/administration.cpp
    src/modules/botowner.h src/modules/botowner.cpp
    src/modules/config.h src/modules/config.cpp
    src/modules/crime.h src/modules/crime.cpp
    src/modules/economy.h src/modules/economy.cpp
    src/modules/fun.h src/modules/fun.cpp
    src/modules/gambling.h src/modules/gambling.cpp
    src/modules/gangs.h src/modules/gangs.cpp
    src/modules/general.h src/modules/general.cpp
    src/modules/goods.h src/modules/goods.cpp
    src/modules/investments.h src/modules/investments.cpp
    src/modules/moderation.h src/modules/moderation.cpp
    src/modules/prestige.h src/modules/prestige.cpp
    src/modules/tasks.h src/modules/tasks.cpp
    src/modules/weapons.h src/modules/weapons.cpp
    src/paginators/leaderboardpaginator.h src/paginators/leaderboardpaginator.cpp
    src/readers/cashtypereader.h src/readers/cashtypereader.cpp
    src/readers/rrguildmembertypereader.h src/readers/rrguildmembertypereader.cpp
    src/systems/filtersystem.h src/systems/filtersystem.cpp
    src/systems/itemsystem.h src/systems/itemsystem.cpp
    src/systems/monitorsystem.h src/systems/monitorsystem.cpp
    src/utils/dpp.h src/utils/dpp.cpp
    src/utils/ld.h src/utils/ld.cpp
    src/utils/random.h
    src/utils/regex.h
    src/utils/stringhash.h
    src/utils/strings.h src/utils/strings.cpp
    src/utils/timestamp.h src/utils/timestamp.cpp
    src/utils/views_concat/concat.hpp
    src/utils/views_concat/utils.hpp)

find_package(Boost REQUIRED COMPONENTS locale url)

find_package(ICU COMPONENTS i18n uc QUIET)
if(ICU_FOUND)
    target_compile_definitions(RRBot PRIVATE RRBOT_HAS_ICU)
    target_include_directories(RRBot PRIVATE ${ICU_INCLUDE_DIRS})
    target_link_libraries(RRBot PRIVATE ${ICU_LIBRARIES})
else()
    message(STATUS "ICU not found. Filtering will be less secure - you should probably just use Discord's built-in system.")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

target_include_directories(RRBot PRIVATE src ${Boost_INCLUDE_DIRS} ${MONGOCXX_INCLUDE_DIRS})
target_link_libraries(RRBot PRIVATE ${Boost_LIBRARIES} ${MONGOCXX_LIBRARIES} dppcmd dppinteract)

include(GNUInstallDirs)
install(TARGETS RRBot
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
