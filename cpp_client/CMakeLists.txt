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
option(USE_SYSTEM_LIBS "Use system libraries instead of bundled" OFF)

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

# GLAD (OpenGL loader)
set(GLAD_SOURCES
    external/glad/src/glad.c
)

# nlohmann/json (header-only JSON library)
include_directories(external/json/include)

# STB (header-only image loading library)
include_directories(external/stb)

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
    src/network/tcp_client.cpp
    src/network/protocol_handler.cpp
    src/ui/hud.cpp
    src/ui/input_handler.cpp
    ${GLAD_SOURCES}
)

set(CLIENT_HEADERS
    include/core/application.h
    include/core/game_client.h
    include/core/embedded_server.h
    include/core/session_manager.h
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
    include/network/tcp_client.h
    include/network/protocol_handler.h
    include/ui/hud.h
    include/ui/input_handler.h
)

# Main executable
add_executable(eve_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

# Link libraries
target_link_libraries(eve_client 
    Threads::Threads
    OpenGL::GL
    glfw
)

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
        ${GLAD_SOURCES}
    )
    
    target_link_libraries(test_lighting
        Threads::Threads
        OpenGL::GL
        glfw
    )
    
    if(WIN32)
        target_link_libraries(test_lighting ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_lighting dl X11)
    elseif(APPLE)
        target_link_libraries(test_lighting dl)
        target_link_libraries(test_lighting "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
endif()

message(STATUS "")
message(STATUS "EVE OFFLINE Client Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenGL: ${OPENGL_VERSION}")
message(STATUS "  Use System Libraries: ${USE_SYSTEM_LIBS}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")
