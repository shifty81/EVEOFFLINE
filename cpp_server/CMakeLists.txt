cmake_minimum_required(VERSION 3.15)
project(AtlasDedicatedServer VERSION 1.0.0 LANGUAGES CXX)

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
    src/utils/logger.cpp
    src/utils/server_metrics.cpp
    src/ui/server_console.cpp
    src/ecs/entity.cpp
    src/ecs/world.cpp
    src/systems/movement_system.cpp
    src/systems/combat_system.cpp
    src/systems/ai_system.cpp
    src/systems/targeting_system.cpp
    src/systems/capacitor_system.cpp
    src/systems/shield_recharge_system.cpp
    src/systems/weapon_system.cpp
    src/systems/wormhole_system.cpp
    src/systems/fleet_system.cpp
    src/systems/mission_system.cpp
    src/systems/skill_system.cpp
    src/systems/module_system.cpp
    src/systems/inventory_system.cpp
    src/systems/loot_system.cpp
    src/systems/insurance_system.cpp
    src/systems/captain_personality_system.cpp
    src/systems/corporation_system.cpp
    src/systems/bounty_system.cpp
    src/systems/market_system.cpp
    src/systems/drone_system.cpp
    src/systems/contract_system.cpp
    src/systems/pi_system.cpp
    src/systems/manufacturing_system.cpp
    src/systems/research_system.cpp
    src/systems/chat_system.cpp
    src/systems/character_creation_system.cpp
    src/systems/tournament_system.cpp
    src/systems/leaderboard_system.cpp
    src/systems/station_system.cpp
    src/systems/wreck_salvage_system.cpp
    src/systems/fleet_morale_system.cpp
    src/systems/fleet_chatter_system.cpp
    src/systems/warp_anomaly_system.cpp
    src/systems/captain_relationship_system.cpp
    src/systems/emotional_arc_system.cpp
    src/systems/fleet_cargo_system.cpp
    src/systems/tactical_overlay_system.cpp
    src/systems/mining_system.cpp
    src/systems/fleet_formation_system.cpp
    src/systems/captain_memory_system.cpp
    src/systems/ship_fitting_system.cpp
    src/systems/warp_cinematic_system.cpp
    src/systems/refining_system.cpp
    src/systems/anomaly_system.cpp
    src/systems/reputation_system.cpp
    src/systems/scanner_system.cpp
    src/systems/difficulty_scaling_system.cpp
    src/systems/mission_template_system.cpp
    src/systems/mission_generator_system.cpp
    src/data/ship_database.cpp
    src/data/npc_database.cpp
    src/data/wormhole_database.cpp
    src/data/world_persistence.cpp
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
    include/utils/logger.h
    include/utils/server_metrics.h
    include/ui/server_console.h
    include/ecs/component.h
    include/ecs/entity.h
    include/ecs/system.h
    include/ecs/world.h
    include/components/game_components.h
    include/systems/movement_system.h
    include/systems/combat_system.h
    include/systems/ai_system.h
    include/systems/targeting_system.h
    include/systems/capacitor_system.h
    include/systems/shield_recharge_system.h
    include/systems/weapon_system.h
    include/systems/wormhole_system.h
    include/systems/fleet_system.h
    include/systems/mission_system.h
    include/systems/skill_system.h
    include/systems/module_system.h
    include/systems/inventory_system.h
    include/systems/loot_system.h
    include/systems/insurance_system.h
    include/systems/captain_personality_system.h
    include/systems/corporation_system.h
    include/systems/bounty_system.h
    include/systems/market_system.h
    include/systems/drone_system.h
    include/systems/contract_system.h
    include/systems/pi_system.h
    include/systems/manufacturing_system.h
    include/systems/research_system.h
    include/systems/chat_system.h
    include/systems/character_creation_system.h
    include/systems/tournament_system.h
    include/systems/leaderboard_system.h
    include/systems/station_system.h
    include/systems/wreck_salvage_system.h
    include/systems/fleet_morale_system.h
    include/systems/fleet_chatter_system.h
    include/systems/warp_anomaly_system.h
    include/systems/captain_relationship_system.h
    include/systems/emotional_arc_system.h
    include/systems/fleet_cargo_system.h
    include/systems/tactical_overlay_system.h
    include/systems/mining_system.h
    include/systems/fleet_formation_system.h
    include/systems/captain_memory_system.h
    include/systems/ship_fitting_system.h
    include/systems/warp_cinematic_system.h
    include/systems/refining_system.h
    include/systems/anomaly_system.h
    include/systems/reputation_system.h
    include/systems/scanner_system.h
    include/systems/difficulty_scaling_system.h
    include/systems/mission_template_system.h
    include/systems/mission_generator_system.h
    include/data/ship_database.h
    include/data/npc_database.h
    include/data/wormhole_database.h
    include/data/world_persistence.h
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
add_executable(atlas_dedicated_server ${SERVER_SOURCES} ${SERVER_HEADERS})

# Link libraries
target_link_libraries(atlas_dedicated_server 
    Threads::Threads
)

if(USE_STEAM_SDK AND STEAM_LIBRARIES)
    target_link_libraries(atlas_dedicated_server ${STEAM_LIBRARIES})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atlas_dedicated_server ws2_32)
elseif(UNIX)
    target_link_libraries(atlas_dedicated_server dl)
endif()

# Installation
install(TARGETS atlas_dedicated_server
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
message(STATUS "Atlas Dedicated Server Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Steam Integration: ${USE_STEAM_SDK}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")

# Test executable
if(BUILD_TESTS)
    # Sources needed by test_systems (exclude main.cpp, server.cpp, game_session.cpp)
    set(TEST_SUPPORT_SOURCES
        src/ecs/entity.cpp
        src/ecs/world.cpp
        src/systems/capacitor_system.cpp
        src/systems/shield_recharge_system.cpp
        src/systems/weapon_system.cpp
        src/systems/combat_system.cpp
        src/systems/targeting_system.cpp
        src/systems/movement_system.cpp
        src/systems/ai_system.cpp
        src/data/ship_database.cpp
        src/data/wormhole_database.cpp
        src/data/npc_database.cpp
        src/systems/wormhole_system.cpp
        src/systems/fleet_system.cpp
        src/systems/mission_system.cpp
        src/systems/skill_system.cpp
        src/systems/module_system.cpp
        src/systems/inventory_system.cpp
        src/systems/loot_system.cpp
        src/systems/insurance_system.cpp
        src/systems/captain_personality_system.cpp
        src/systems/corporation_system.cpp
        src/systems/bounty_system.cpp
        src/systems/market_system.cpp
        src/systems/drone_system.cpp
        src/systems/contract_system.cpp
        src/systems/pi_system.cpp
        src/systems/manufacturing_system.cpp
        src/systems/research_system.cpp
        src/systems/chat_system.cpp
        src/systems/character_creation_system.cpp
        src/systems/tournament_system.cpp
        src/systems/leaderboard_system.cpp
        src/systems/station_system.cpp
        src/systems/wreck_salvage_system.cpp
        src/systems/fleet_morale_system.cpp
        src/systems/fleet_chatter_system.cpp
        src/systems/warp_anomaly_system.cpp
        src/systems/captain_relationship_system.cpp
        src/systems/emotional_arc_system.cpp
        src/systems/fleet_cargo_system.cpp
        src/systems/tactical_overlay_system.cpp
        src/systems/mining_system.cpp
        src/systems/fleet_formation_system.cpp
        src/systems/captain_memory_system.cpp
        src/systems/ship_fitting_system.cpp
        src/systems/warp_cinematic_system.cpp
        src/systems/refining_system.cpp
        src/systems/anomaly_system.cpp
        src/systems/reputation_system.cpp
        src/systems/scanner_system.cpp
        src/systems/difficulty_scaling_system.cpp
        src/systems/mission_template_system.cpp
        src/systems/mission_generator_system.cpp
        src/data/world_persistence.cpp
        src/utils/logger.cpp
        src/utils/server_metrics.cpp
        src/ui/server_console.cpp
        src/server.cpp
        src/game_session.cpp
        src/network/tcp_server.cpp
        src/network/protocol_handler.cpp
        src/config/server_config.cpp
        src/auth/steam_auth.cpp
        src/auth/whitelist.cpp
        src/utils/name_generator.cpp
        src/data/ship_database.cpp
    )

    add_executable(test_systems
        test_systems.cpp
        ${TEST_SUPPORT_SOURCES}
    )
    target_link_libraries(test_systems Threads::Threads)
    if(WIN32)
        target_link_libraries(test_systems ws2_32)
    endif()
endif()
