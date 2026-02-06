cmake_minimum_required(VERSION 3.15)
project(EVEOfflineDedicatedServer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(USE_STEAM_SDK "Enable Steam integration" ON)
option(BUILD_TESTS "Build test suite" ON)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WIN32)
elseif(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/server.cpp
    src/game_session.cpp
    src/network/tcp_server.cpp
    src/network/protocol_handler.cpp
    src/config/server_config.cpp
    src/auth/steam_auth.cpp
    src/auth/whitelist.cpp
    src/utils/name_generator.cpp
    src/ecs/entity.cpp
    src/ecs/world.cpp
    src/systems/movement_system.cpp
    src/systems/combat_system.cpp
    src/systems/ai_system.cpp
    src/systems/targeting_system.cpp
)

set(SERVER_HEADERS
    include/server.h
    include/game_session.h
    include/network/tcp_server.h
    include/network/protocol_handler.h
    include/config/server_config.h
    include/auth/steam_auth.h
    include/auth/whitelist.h
    include/utils/name_generator.h
    include/ecs/component.h
    include/ecs/entity.h
    include/ecs/system.h
    include/ecs/world.h
    include/components/game_components.h
    include/systems/movement_system.h
    include/systems/combat_system.h
    include/systems/ai_system.h
    include/systems/targeting_system.h
)

# Steam SDK configuration
if(USE_STEAM_SDK)
    # Path to Steamworks SDK (users need to download this separately)
    set(STEAM_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/steamworks_sdk" CACHE PATH "Path to Steamworks SDK")
    
    if(EXISTS "${STEAM_SDK_PATH}/public/steam/steam_api.h")
        message(STATUS "Found Steamworks SDK at: ${STEAM_SDK_PATH}")
        add_definitions(-DUSE_STEAM)
        include_directories(${STEAM_SDK_PATH}/public)
        
        # Link Steam library
        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                link_directories(${STEAM_SDK_PATH}/redistributable_bin/win64)
            else()
                link_directories(${STEAM_SDK_PATH}/redistributable_bin)
            endif()
            set(STEAM_LIBRARIES steam_api64)
        elseif(LINUX)
            link_directories(${STEAM_SDK_PATH}/redistributable_bin/linux64)
            set(STEAM_LIBRARIES steam_api)
        endif()
    else()
        message(WARNING "Steamworks SDK not found. Steam integration will be disabled.")
        message(WARNING "Download SDK from: https://partner.steamgames.com/")
        set(USE_STEAM_SDK OFF)
    endif()
endif()

# Main executable
add_executable(eve_dedicated_server ${SERVER_SOURCES} ${SERVER_HEADERS})

# Link libraries
target_link_libraries(eve_dedicated_server 
    Threads::Threads
)

if(USE_STEAM_SDK AND STEAM_LIBRARIES)
    target_link_libraries(eve_dedicated_server ${STEAM_LIBRARIES})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(eve_dedicated_server ws2_32)
elseif(UNIX)
    target_link_libraries(eve_dedicated_server dl)
endif()

# Installation
install(TARGETS eve_dedicated_server
    RUNTIME DESTINATION bin
)

install(DIRECTORY config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.json" PATTERN "*.cfg"
)

# Copy configuration files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

message(STATUS "")
message(STATUS "EVE OFFLINE Dedicated Server Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Steam Integration: ${USE_STEAM_SDK}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")
