set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPURE_BIOS_ONLY")

message("Flags: ${CMAKE_CXX_FLAGS}")

# Source code
file(GLOB_RECURSE SOURCES
        main.cpp
        Parameters.cpp
        Parameters.h
        server/IRouter.h
        server/handlers/RequestHandler.cpp
        server/handlers/RequestHandler.h
        server/handlers/RequestHandlerTools.cpp
        server/handlers/RequestHandlerTools.h
        server/handlers/Mime.cpp
        server/handlers/Mime.h
        server/Server.cpp
        server/Server.h
)

# ES dependencies
file(GLOB_RECURSE DEPENDENCIES
    ../es-core/src/utils/String.cpp
    ../es-core/src/utils/Files.cpp
    ../es-core/src/utils/os/fs/Path.cpp
    ../es-core/src/utils/Log.cpp
    ../es-core/src/utils/os/system/Thread.cpp
    ../es-core/src/utils/os/system/Mutex.cpp
    ../es-core/src/utils/datetime/DateTime.cpp
    ../es-core/src/utils/datetime/TimeSpan.cpp
    ../es-core/src/utils/storage/*.h
    ../es-core/src/utils/json/JSONBuilder.h
    ../es-core/src/RootFolders.cpp
    ../es-core/src/RootFolders.h
)

include_directories(
        .
        ../es-core/src
        ../es-app/src
        ../external
        ../external/pistache/include
)

add_executable(
        webserver
        ${SOURCES}
        ${DEPENDENCIES}
)

target_link_libraries(webserver pistache_static zip pugixml)

#-------------------------------------------------------------------------------
# set up install stuff so `make install` does something useful
install(TARGETS webserver
        RUNTIME
        DESTINATION bin)