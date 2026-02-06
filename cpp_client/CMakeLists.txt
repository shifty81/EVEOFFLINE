cmake_minimum_required(VERSION 3.15)
project(EVEOfflineClient VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(USE_SYSTEM_LIBS "Use system libraries instead of bundled" ON)  # Default to ON since they're available

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WIN32)
elseif(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
elseif(APPLE)
    add_definitions(-DMACOS)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# OpenAL (optional audio support)
find_package(OpenAL QUIET)
if(OpenAL_FOUND)
    message(STATUS "OpenAL found - audio support enabled")
    add_definitions(-DUSE_OPENAL)
    set(AUDIO_SOURCES
        src/audio/audio_manager.cpp
        src/audio/audio_generator.cpp
    )
    set(AUDIO_HEADERS
        include/audio/audio_manager.h
        include/audio/audio_generator.h
    )
else()
    message(STATUS "OpenAL not found - audio support disabled")
    set(AUDIO_SOURCES "")
    set(AUDIO_HEADERS "")
endif()

# GLEW (OpenGL extension wrangler) - Optional, falls back to GLAD if not found
if(USE_SYSTEM_LIBS)
    find_package(GLEW QUIET)
    if(GLEW_FOUND)
        message(STATUS "GLEW found - using system GLEW")
        set(USE_GLEW ON)
    else()
        message(WARNING "GLEW not found!")
        message(WARNING "")
        message(WARNING "Please install GLEW via vcpkg:")
        message(WARNING "  vcpkg install glew:x64-windows")
        message(WARNING "")
        message(WARNING "Or install all dependencies at once:")
        message(WARNING "  vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows")
        message(WARNING "")
        message(WARNING "Then re-run CMake with the toolchain file:")
        message(WARNING "  cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        message(WARNING "")
        message(FATAL_ERROR "GLEW is required when USE_SYSTEM_LIBS=ON. See above for installation instructions.")
    endif()
else()
    set(USE_GLEW OFF)
endif()

# GLFW
if(USE_SYSTEM_LIBS)
    find_package(glfw3 3.3 REQUIRED)
else()
    message(STATUS "Using bundled GLFW")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/glfw EXCLUDE_FROM_ALL)
endif()

# GLM (header-only)
if(USE_SYSTEM_LIBS)
    find_package(glm REQUIRED)
else()
    message(STATUS "Using bundled GLM")
    include_directories(external/glm)
endif()

# GLAD (OpenGL loader) - Always include unless using GLEW
if(USE_GLEW)
    message(STATUS "Using system GLEW (GLAD not needed)")
    set(GLAD_SOURCES "")
else()
    message(STATUS "Using bundled GLAD for OpenGL loading")
    set(GLAD_SOURCES
        external/glad/src/glad.c
    )
endif()

# nlohmann/json (header-only JSON library)
if(USE_SYSTEM_LIBS)
    find_package(nlohmann_json QUIET)
    if(nlohmann_json_FOUND)
        message(STATUS "Using system nlohmann/json")
    else()
        message(STATUS "System nlohmann/json not found, using bundled version")
        include_directories(external/json/include)
    endif()
else()
    message(STATUS "Using bundled nlohmann/json")
    include_directories(external/json/include)
endif()

# STB (header-only image loading library)
include_directories(external/stb)

# ImGui (UI library)
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_impl_glfw.cpp
    external/imgui/imgui_impl_opengl3.cpp
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${OPENGL_INCLUDE_DIR}
)

# Source files
set(CLIENT_SOURCES
    src/main.cpp
    src/core/application.cpp
    src/core/game_client.cpp
    src/core/embedded_server.cpp
    src/core/session_manager.cpp
    src/core/entity.cpp
    src/core/entity_manager.cpp
    src/core/entity_message_parser.cpp
    src/rendering/window.cpp
    src/rendering/shader.cpp
    src/rendering/camera.cpp
    src/rendering/renderer.cpp
    src/rendering/mesh.cpp
    src/rendering/model.cpp
    src/rendering/texture.cpp
    src/rendering/particle_system.cpp
    src/rendering/healthbar_renderer.cpp
    src/rendering/visual_effects.cpp
    src/rendering/pbr_materials.cpp
    src/rendering/lod_manager.cpp
    src/rendering/frustum_culler.cpp
    src/rendering/instanced_renderer.cpp
    src/rendering/asteroid_field_renderer.cpp
    src/rendering/lighting.cpp
    src/rendering/shadow_map.cpp
    src/rendering/gbuffer.cpp
    src/rendering/post_processing.cpp
    src/network/tcp_client.cpp
    src/network/protocol_handler.cpp
    src/network/network_manager.cpp
    src/ui/hud.cpp
    src/ui/input_handler.cpp
    src/ui/ui_manager.cpp
    src/ui/eve_panels.cpp
    src/ui/eve_target_list.cpp
    src/ui/entity_picker.cpp
    src/ui/inventory_panel.cpp
    src/ui/fitting_panel.cpp
    src/ui/mission_panel.cpp
    src/ui/module_browser_panel.cpp
    src/ui/market_panel.cpp
    src/ui/overview_panel.cpp
    ${GLAD_SOURCES}
    ${IMGUI_SOURCES}
    ${AUDIO_SOURCES}
)

set(CLIENT_HEADERS
    include/core/application.h
    include/core/game_client.h
    include/core/embedded_server.h
    include/core/session_manager.h
    include/core/entity.h
    include/core/entity_manager.h
    include/core/entity_message_parser.h
    include/rendering/window.h
    include/rendering/shader.h
    include/rendering/camera.h
    include/rendering/renderer.h
    include/rendering/mesh.h
    include/rendering/model.h
    include/rendering/texture.h
    include/rendering/particle_system.h
    include/rendering/healthbar_renderer.h
    include/rendering/visual_effects.h
    include/rendering/pbr_materials.h
    include/rendering/lod_manager.h
    include/rendering/frustum_culler.h
    include/rendering/instanced_renderer.h
    include/rendering/asteroid_field_renderer.h
    include/rendering/lighting.h
    include/rendering/shadow_map.h
    include/rendering/gbuffer.h
    include/rendering/post_processing.h
    include/network/tcp_client.h
    include/network/protocol_handler.h
    include/network/network_manager.h
    include/ui/hud.h
    include/ui/input_handler.h
    include/ui/ui_manager.h
    include/ui/eve_panels.h
    include/ui/eve_target_list.h
    include/ui/entity_picker.h
    include/ui/inventory_panel.h
    include/ui/fitting_panel.h
    include/ui/mission_panel.h
    include/ui/module_browser_panel.h
    include/ui/market_panel.h
    ${AUDIO_HEADERS}
)

# Main executable
add_executable(eve_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

# Link libraries
target_link_libraries(eve_client 
    Threads::Threads
    OpenGL::GL
    glfw
)

# Link GLEW if available
if(USE_GLEW)
    target_link_libraries(eve_client GLEW::GLEW)
endif()

# Link OpenAL if available
if(OpenAL_FOUND)
    target_link_libraries(eve_client ${OPENAL_LIBRARY})
    target_include_directories(eve_client PRIVATE ${OPENAL_INCLUDE_DIR})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(eve_client ws2_32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(eve_client dl X11)
elseif(APPLE)
    target_link_libraries(eve_client dl)
    target_link_libraries(eve_client "-framework Cocoa -framework IOKit -framework CoreVideo")
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/shaders)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/assets)

# Installation
install(TARGETS eve_client
    RUNTIME DESTINATION bin
)

install(DIRECTORY shaders/
    DESTINATION shaders
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag" PATTERN "*.glsl"
)

install(DIRECTORY assets/
    DESTINATION assets
)

# Tests
if(BUILD_TESTS)
    message(STATUS "Building test suite...")
    
    # Test: Asteroid Field Rendering
    add_executable(test_asteroid_field
        test_asteroid_field.cpp
        src/rendering/window.cpp
        src/rendering/shader.cpp
        src/rendering/camera.cpp
        src/rendering/mesh.cpp
        src/rendering/instanced_renderer.cpp
        src/rendering/asteroid_field_renderer.cpp
        src/ui/input_handler.cpp
        ${GLAD_SOURCES}
    )
    
    target_link_libraries(test_asteroid_field
        Threads::Threads
        OpenGL::GL
        glfw
    )
    
    if(WIN32)
        target_link_libraries(test_asteroid_field ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_asteroid_field dl X11)
    elseif(APPLE)
        target_link_libraries(test_asteroid_field dl)
        target_link_libraries(test_asteroid_field "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: Dynamic Lighting System
    add_executable(test_lighting
        test_lighting.cpp
        src/rendering/window.cpp
        src/rendering/shader.cpp
        src/rendering/camera.cpp
        src/rendering/mesh.cpp
        src/rendering/model.cpp
        src/rendering/lighting.cpp
        src/ui/input_handler.cpp
        src/ui/ui_manager.cpp
        src/ui/eve_panels.cpp
        ${GLAD_SOURCES}
        ${IMGUI_SOURCES}
    )
    
    target_link_libraries(test_lighting
        Threads::Threads
        OpenGL::GL
        glfw
    )
    
    if(USE_GLEW)
        target_link_libraries(test_lighting GLEW::GLEW)
    endif()
    
    if(WIN32)
        target_link_libraries(test_lighting ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_lighting dl X11)
    elseif(APPLE)
        target_link_libraries(test_lighting dl)
        target_link_libraries(test_lighting "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: Shadow Mapping
    add_executable(test_shadow_mapping
        test_shadow_mapping.cpp
        src/rendering/window.cpp
        src/rendering/shader.cpp
        src/rendering/camera.cpp
        src/rendering/mesh.cpp
        src/rendering/shadow_map.cpp
        src/rendering/lighting.cpp
        src/ui/input_handler.cpp
        ${GLAD_SOURCES}
    )
    
    target_link_libraries(test_shadow_mapping
        Threads::Threads
        OpenGL::GL
        glfw
    )
    
    if(WIN32)
        target_link_libraries(test_shadow_mapping ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_shadow_mapping dl X11)
    elseif(APPLE)
        target_link_libraries(test_shadow_mapping dl)
        target_link_libraries(test_shadow_mapping "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: Deferred Rendering
    add_executable(test_deferred_rendering
        test_deferred_rendering.cpp
        src/rendering/window.cpp
        src/rendering/shader.cpp
        src/rendering/camera.cpp
        src/rendering/mesh.cpp
        src/rendering/gbuffer.cpp
        src/rendering/lighting.cpp
        src/ui/input_handler.cpp
        ${GLAD_SOURCES}
    )
    
    target_link_libraries(test_deferred_rendering
        Threads::Threads
        OpenGL::GL
        glfw
    )
    
    if(WIN32)
        target_link_libraries(test_deferred_rendering ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_deferred_rendering dl X11)
    elseif(APPLE)
        target_link_libraries(test_deferred_rendering dl)
        target_link_libraries(test_deferred_rendering "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: UI System
    add_executable(test_ui_system
        test_ui_system.cpp
        src/rendering/window.cpp
        src/ui/ui_manager.cpp
        src/ui/eve_panels.cpp
        ${IMGUI_SOURCES}
    )
    target_link_libraries(test_ui_system
        Threads::Threads
        OpenGL::GL
        glfw
    )
    if(USE_GLEW)
        target_link_libraries(test_ui_system GLEW::GLEW)
    endif()
    if(WIN32)
        target_link_libraries(test_ui_system ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_ui_system dl X11)
    elseif(APPLE)
        target_link_libraries(test_ui_system dl)
        target_link_libraries(test_ui_system "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: Post-Processing Effects
    add_executable(test_post_processing
        test_post_processing.cpp
        src/rendering/window.cpp
        src/rendering/shader.cpp
        src/rendering/camera.cpp
        src/rendering/lighting.cpp
        src/rendering/post_processing.cpp
        ${GLAD_SOURCES}
    )
    target_link_libraries(test_post_processing
        Threads::Threads
        OpenGL::GL
        glfw
    )
    if(WIN32)
        target_link_libraries(test_post_processing ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_post_processing dl X11)
    elseif(APPLE)
        target_link_libraries(test_post_processing dl)
        target_link_libraries(test_post_processing "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    
    # Test: Audio System (if OpenAL is available)
    if(OpenAL_FOUND)
        add_executable(test_audio_system
            test_audio_system.cpp
            ${AUDIO_SOURCES}
        )
        target_link_libraries(test_audio_system
            Threads::Threads
            ${OPENAL_LIBRARY}
        )
        target_include_directories(test_audio_system PRIVATE ${OPENAL_INCLUDE_DIR})
        
        if(WIN32)
            target_link_libraries(test_audio_system ws2_32)
        elseif(UNIX AND NOT APPLE)
            target_link_libraries(test_audio_system dl)
        elseif(APPLE)
            target_link_libraries(test_audio_system dl)
        endif()
    endif()
    
    # Test: Network System
    add_executable(test_network
        test_network.cpp
        src/network/tcp_client.cpp
        src/network/protocol_handler.cpp
        src/network/network_manager.cpp
    )
    target_link_libraries(test_network
        Threads::Threads
    )
    if(WIN32)
        target_link_libraries(test_network ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_network)
    elseif(APPLE)
        target_link_libraries(test_network)
    endif()
    
    # Test: Entity Synchronization
    add_executable(test_entity_sync
        test_entity_sync.cpp
        src/core/entity.cpp
        src/core/entity_manager.cpp
        src/core/entity_message_parser.cpp
    )
    target_link_libraries(test_entity_sync
        Threads::Threads
    )
    if(WIN32)
        target_link_libraries(test_entity_sync ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_entity_sync)
    elseif(APPLE)
        target_link_libraries(test_entity_sync)
    endif()
    
    # Test: Enhanced UI (Phase 4.5)
    add_executable(test_enhanced_ui
        test_enhanced_ui.cpp
        ${CLIENT_SOURCES}
        ${CLIENT_HEADERS}
    )
    target_link_libraries(test_enhanced_ui ${LINK_LIBS})
    
    # Test: Phase 4.6 Advanced Features
    add_executable(test_phase46_advanced
        test_phase46_advanced.cpp
        ${CLIENT_SOURCES}
        ${CLIENT_HEADERS}
    )
    target_link_libraries(test_phase46_advanced ${LINK_LIBS})
    
    # Test: Phase 4.8 Server Response Handling
    add_executable(test_server_responses
        test_server_responses.cpp
        src/network/tcp_client.cpp
        src/network/protocol_handler.cpp
        src/network/network_manager.cpp
    )
    target_link_libraries(test_server_responses
        Threads::Threads
    )
    if(WIN32)
        target_link_libraries(test_server_responses ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_server_responses)
    elseif(APPLE)
        target_link_libraries(test_server_responses)
    endif()
endif()

message(STATUS "")
message(STATUS "EVE OFFLINE Client Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenGL: ${OPENGL_VERSION}")
message(STATUS "  Use System Libraries: ${USE_SYSTEM_LIBS}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")
