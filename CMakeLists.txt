cmake_minimum_required(VERSION 3.5)
project(RRBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_COROUTINES "" ON)
add_subdirectory(dpp-command-handler)

add_executable(RRBot
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
    src/entities/goods/item.h
    src/entities/goods/tool.h
    src/modules/administration.h src/modules/administration.cpp
    src/modules/botowner.h src/modules/botowner.cpp
    src/modules/config.h src/modules/config.cpp
    src/modules/crime.h src/modules/crime.cpp
    src/modules/economy.h src/modules/economy.cpp
    src/modules/fun.h src/modules/fun.cpp
    src/modules/gambling.h src/modules/gambling.cpp
    src/modules/general.h src/modules/general.cpp
    src/readers/cashtypereader.h src/readers/cashtypereader.cpp
    src/utils/dpp.h src/utils/dpp.cpp
    src/utils/ld.h src/utils/ld.cpp
    src/utils/random.h
    src/utils/ranges.h
    src/utils/strings.h src/utils/strings.cpp
    src/utils/timestamp.h src/utils/timestamp.cpp)

find_package(Boost COMPONENTS locale)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

target_include_directories(RRBot PRIVATE src ${Boost_INCLUDE_DIRS} ${MONGOCXX_INCLUDE_DIRS})
target_link_libraries(RRBot PRIVATE ${Boost_LIBRARIES} ${MONGOCXX_LIBRARIES} dpp-command-handler)

include(GNUInstallDirs)
install(TARGETS RRBot
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
