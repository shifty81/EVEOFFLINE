cmake_minimum_required(VERSION 3.15)
project(AtlasClient VERSION 1.0.0 LANGUAGES C CXX)

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
option(USE_RMLUI "Enable RmlUi for advanced Photon UI game panels" ON)


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
    # Fix OpenAL include path: ensure <AL/al.h> works
    # Some FindOpenAL modules set OPENAL_INCLUDE_DIR to the AL/ subdirectory itself
    if(EXISTS "${OPENAL_INCLUDE_DIR}/al.h" AND NOT EXISTS "${OPENAL_INCLUDE_DIR}/AL/al.h")
        get_filename_component(OPENAL_INCLUDE_PARENT "${OPENAL_INCLUDE_DIR}" DIRECTORY)
        set(OPENAL_INCLUDE_DIR "${OPENAL_INCLUDE_PARENT}")
    endif()
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
        message(STATUS "GLEW not found - falling back to bundled GLAD")
        message(STATUS "")
        message(STATUS "Consider installing GLEW via vcpkg:")
        message(STATUS "  vcpkg install glew:x64-windows")
        message(STATUS "")
        message(STATUS "Or install all dependencies at once:")
        message(STATUS "  vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows")
        message(STATUS "")
        set(USE_GLEW OFF)
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
    # Check if GLAD source exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c")
        set(GLAD_SOURCES external/glad/src/glad.c)
    else()
        message(FATAL_ERROR 
            "GLAD source not found at external/glad/src/glad.c\n"
            "Either:\n"
            "  1. Install GLEW via vcpkg: vcpkg install glew:x64-windows\n"
            "     Then reconfigure with -DUSE_SYSTEM_LIBS=ON\n"
            "  2. Or ensure external/glad directory is populated")
    endif()
endif()

# nlohmann/json (header-only JSON library)
if(USE_SYSTEM_LIBS)
    find_package(nlohmann_json QUIET)
    if(nlohmann_json_FOUND)
        message(STATUS "Using system nlohmann/json")
    else()
        message(STATUS "System nlohmann/json not found, using bundled version")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann")
            include_directories(external/nlohmann)
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/json/include")
            include_directories(external/json/include)
        else()
            message(WARNING "nlohmann/json not found. Install via: vcpkg install nlohmann-json:x64-windows")
        endif()
    endif()
else()
    message(STATUS "Using bundled nlohmann/json")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann")
        include_directories(external/nlohmann)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/json/include")
        include_directories(external/json/include)
    else()
        message(FATAL_ERROR "Bundled nlohmann/json not found at external/nlohmann or external/json/include")
    endif()
endif()

# STB (header-only image loading library)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/stb")
    include_directories(external/stb)
    message(STATUS "Using bundled STB image loader")
else()
    message(WARNING "STB image loader not found at external/stb - texture loading may fail")
endif()

# tinyobjloader (header-only .obj model loader)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader")
    include_directories(external/tinyobjloader)
    message(STATUS "Using bundled tinyobjloader for .obj model loading")
else()
    message(WARNING "tinyobjloader not found at external/tinyobjloader - .obj loading disabled")
endif()

# tinygltf (header-only .gltf/.glb model loader)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf")
    include_directories(external/tinygltf)
    # Tell tinygltf to NOT include json.hpp itself - we'll include nlohmann/json before tinygltf
    add_definitions(-DTINYGLTF_NO_INCLUDE_JSON)
    # Disable stb_image_write dependency (only stb_image.h for loading is available)
    add_definitions(-DTINYGLTF_NO_STB_IMAGE_WRITE)
    message(STATUS "Using bundled tinygltf for .gltf/.glb model loading")
else()
    message(WARNING "tinygltf not found at external/tinygltf - .gltf/.glb loading disabled")
endif()


# RmlUi (advanced UI framework — optional, used for game panels)
# See docs/design/UI_FRAMEWORK_EVALUATION.md for rationale.
# Uses RmlUi's official GLFW platform and OpenGL 3 renderer backends.
set(RMLUI_AVAILABLE OFF)
set(RMLUI_SOURCES "")
set(RMLUI_BACKEND_SOURCES "")
if(USE_RMLUI)
    find_package(RmlUi QUIET)
    if(RmlUi_FOUND)
        message(STATUS "RmlUi found via system/vcpkg — advanced Photon UI enabled")
        set(RMLUI_AVAILABLE ON)
    else()
        # FetchContent fallback: clone from GitHub
        include(FetchContent)
        FetchContent_Declare(
            RmlUi
            GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
            GIT_TAG        6.0
            GIT_SHALLOW    TRUE
        )
        # Build RmlUi as a static library; disable samples, tests, etc.
        set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
        set(RMLUI_TESTS OFF CACHE BOOL "" FORCE)
        set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(RmlUi)
        if(TARGET rmlui_core)
            message(STATUS "RmlUi fetched via FetchContent — advanced Photon UI enabled")
            set(RMLUI_AVAILABLE ON)
        else()
            message(WARNING
                "RmlUi not found and FetchContent failed.\n"
                "  Install via vcpkg:  vcpkg install rmlui:x64-windows\n"
                "  Or on Linux:        sudo apt install librmlui-dev\n"
                "  Falling back to stub UI.")
        endif()
    endif()

    # Set up RmlUi backend sources (GLFW platform + GL3 renderer)
    if(RMLUI_AVAILABLE AND NOT RmlUi_FOUND)
        set(RMLUI_BACKEND_DIR "${rmlui_SOURCE_DIR}/Backends")
        set(RMLUI_BACKEND_SOURCES
            ${RMLUI_BACKEND_DIR}/RmlUi_Platform_GLFW.cpp
            ${RMLUI_BACKEND_DIR}/RmlUi_Renderer_GL3.cpp
        )
        # Include the backend headers
        include_directories(${RMLUI_BACKEND_DIR})
        message(STATUS "RmlUi backend sources: GLFW + GL3 from ${RMLUI_BACKEND_DIR}")
    endif()
else()
    message(STATUS "RmlUi disabled (UI stubs only). Enable with -DUSE_RMLUI=ON")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${OPENGL_INCLUDE_DIR}
)


# Only include GLAD headers if not using GLEW
if(NOT USE_GLEW)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)
    endif()
endif()

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
    src/core/file_logger.cpp
    src/rendering/window.cpp
    src/rendering/shader.cpp
    src/rendering/camera.cpp
    src/rendering/renderer.cpp
    src/rendering/mesh.cpp
    src/rendering/model.cpp
    src/rendering/ship_part_library.cpp
    src/rendering/ship_generation_rules.cpp
    src/rendering/procedural_mesh_ops.cpp
    src/rendering/procedural_ship_generator.cpp
    src/rendering/reference_model_analyzer.cpp
    src/rendering/texture.cpp
    src/rendering/particle_system.cpp
    src/rendering/healthbar_renderer.cpp
    src/rendering/visual_effects.cpp
    src/rendering/pbr_materials.cpp
    src/rendering/lod_manager.cpp
    src/rendering/frustum_culler.cpp
    src/rendering/instanced_renderer.cpp
    src/rendering/asteroid_field_renderer.cpp
    src/rendering/station_renderer.cpp
    src/rendering/lighting.cpp
    src/rendering/shadow_map.cpp
    src/rendering/gbuffer.cpp
    src/rendering/post_processing.cpp
    src/network/tcp_client.cpp
    src/network/protocol_handler.cpp
    src/network/network_manager.cpp
    src/ui/input_handler.cpp
    src/ui/entity_picker.cpp
    src/ui/context_menu.cpp
    src/ui/radial_menu.cpp
    src/ui/rml_ui_manager.cpp
    src/ui/atlas/atlas_renderer.cpp
    src/ui/atlas/atlas_context.cpp
    src/ui/atlas/atlas_widgets.cpp
    src/ui/atlas/atlas_hud.cpp
    src/core/solar_system_scene.cpp
    src/core/ship_physics.cpp
    ${GLAD_SOURCES}
    ${AUDIO_SOURCES}
    ${RMLUI_BACKEND_SOURCES}
)



set(CLIENT_HEADERS
    include/core/application.h
    include/core/game_client.h
    include/core/embedded_server.h
    include/core/session_manager.h
    include/core/entity.h
    include/core/entity_manager.h
    include/core/entity_message_parser.h
    include/core/file_logger.h
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
    include/rendering/station_renderer.h
    include/rendering/lighting.h
    include/rendering/shadow_map.h
    include/rendering/gbuffer.h
    include/rendering/post_processing.h
    include/network/tcp_client.h
    include/network/protocol_handler.h
    include/network/network_manager.h
    include/ui/input_handler.h
    include/ui/entity_picker.h
    include/ui/context_menu.h
    include/ui/rml_ui_manager.h
    include/ui/radial_menu.h
    include/ui/layout_manager.h
    include/ui/atlas/atlas_types.h
    include/ui/atlas/atlas_renderer.h
    include/ui/atlas/atlas_context.h
    include/ui/atlas/atlas_widgets.h
    include/ui/atlas/atlas_hud.h
    include/core/solar_system_scene.h
    include/core/ship_physics.h
    ${AUDIO_HEADERS}
)

# Main executable
add_executable(atlas_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

# Link libraries
target_link_libraries(atlas_client 
    Threads::Threads
    OpenGL::GL
    glfw
    glm::glm
)

# Link nlohmann_json if available
if(nlohmann_json_FOUND)
    target_link_libraries(atlas_client nlohmann_json::nlohmann_json)
endif()

# Link GLEW if available
if(USE_GLEW)
    target_link_libraries(atlas_client GLEW::GLEW)
endif()



# Link RmlUi if available
if(RMLUI_AVAILABLE)
    target_compile_definitions(atlas_client PRIVATE USE_RMLUI)
    if(RmlUi_FOUND)
        target_link_libraries(atlas_client RmlUi::RmlUi)
    elseif(TARGET rmlui_core)
        target_link_libraries(atlas_client rmlui_core rmlui_debugger)
    endif()
endif()

# Copy UI resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ui_resources/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/ui_resources)

# Link OpenAL if available
if(OpenAL_FOUND)
    target_link_libraries(atlas_client ${OPENAL_LIBRARY})
    target_include_directories(atlas_client PRIVATE ${OPENAL_INCLUDE_DIR})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atlas_client ws2_32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(atlas_client dl X11)
elseif(APPLE)
    target_link_libraries(atlas_client dl)
    target_link_libraries(atlas_client "-framework Cocoa -framework IOKit -framework CoreVideo")
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/shaders)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/assets)

# Automatically extract OBJ ship models from 7z archive during build
# Models are used instead of procedural generation for ships
set(OBJ_MODELS_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../data/ships/obj_models/obj_models.7z")
set(OBJ_MODELS_DIR "${CMAKE_BINARY_DIR}/bin/models/ships")
if(EXISTS "${OBJ_MODELS_ARCHIVE}")
    find_program(P7ZIP_EXECUTABLE NAMES 7z 7za)
    if(P7ZIP_EXECUTABLE)
        if(NOT EXISTS "${OBJ_MODELS_DIR}")
            message(STATUS "Extracting OBJ ship models from ${OBJ_MODELS_ARCHIVE}...")
            file(MAKE_DIRECTORY "${OBJ_MODELS_DIR}")
            execute_process(
                COMMAND ${P7ZIP_EXECUTABLE} x "${OBJ_MODELS_ARCHIVE}" -o${OBJ_MODELS_DIR} -y
                RESULT_VARIABLE EXTRACT_RESULT
                OUTPUT_QUIET
            )
            if(EXTRACT_RESULT EQUAL 0)
                message(STATUS "OBJ ship models extracted to ${OBJ_MODELS_DIR}")
            else()
                message(WARNING "Failed to extract OBJ ship models (exit code: ${EXTRACT_RESULT})")
            endif()
        else()
            message(STATUS "OBJ ship models already extracted at ${OBJ_MODELS_DIR}")
        endif()
    else()
        message(STATUS "7z not found - install p7zip-full (Linux) or 7-Zip (Windows) to auto-extract ship models")
    endif()
else()
    message(STATUS "OBJ models archive not found at ${OBJ_MODELS_ARCHIVE}")
endif()

# For multi-config generators (e.g., Visual Studio, Xcode) the executable is
# placed in a per-configuration subdirectory (e.g., bin/Release/).  Copy the
# shaders and assets next to the executable so they can be found at runtime.
add_custom_command(TARGET atlas_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:atlas_client>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:atlas_client>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_resources
        $<TARGET_FILE_DIR:atlas_client>/ui_resources
    COMMENT "Copying shaders, assets, and UI resources next to executable"
)

# Copy extracted OBJ models next to executable for multi-config generators.
# Always register this post-build step.  The make_directory call ensures the
# destination exists, and we only copy from OBJ_MODELS_DIR when it is present.
add_custom_command(TARGET atlas_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:atlas_client>/models/ships"
    COMMENT "Ensuring models/ships directory exists next to executable"
)
if(EXISTS "${OBJ_MODELS_DIR}")
    add_custom_command(TARGET atlas_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OBJ_MODELS_DIR}"
            "$<TARGET_FILE_DIR:atlas_client>/models/ships"
        COMMENT "Copying extracted OBJ ship models next to executable"
    )
endif()

# Automatically extract reference models from testing/ archives for the
# ReferenceModelAnalyzer.  These uploaded models are used to learn
# geometric traits (cross-section profiles, aspect ratios, radius
# multipliers) that guide procedural ship generation.
set(TESTING_MODELS_DIR "${CMAKE_BINARY_DIR}/bin/testing_models")
set(TESTING_ARCHIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../testing")
if(EXISTS "${TESTING_ARCHIVE_DIR}" AND P7ZIP_EXECUTABLE)
    if(NOT EXISTS "${TESTING_MODELS_DIR}")
        file(MAKE_DIRECTORY "${TESTING_MODELS_DIR}")
        # Extract spaceship RAR
        set(SPACESHIP_RAR "${TESTING_ARCHIVE_DIR}/99-intergalactic_spaceship-obj.rar")
        if(EXISTS "${SPACESHIP_RAR}")
            message(STATUS "Extracting Intergalactic Spaceship model...")
            execute_process(
                COMMAND ${P7ZIP_EXECUTABLE} x "${SPACESHIP_RAR}" -o${TESTING_MODELS_DIR}/spaceship -y
                RESULT_VARIABLE EXTRACT_RESULT OUTPUT_QUIET
            )
            if(EXTRACT_RESULT EQUAL 0)
                message(STATUS "  Extracted to ${TESTING_MODELS_DIR}/spaceship")
            endif()
        endif()
        # Extract Vulcan Dkyr Class ZIP
        set(VULCAN_ZIP "${TESTING_ARCHIVE_DIR}/qy0sx26192io-VulcanDkyrClass.zip")
        if(EXISTS "${VULCAN_ZIP}")
            message(STATUS "Extracting Vulcan Dkyr Class model...")
            execute_process(
                COMMAND ${P7ZIP_EXECUTABLE} x "${VULCAN_ZIP}" -o${TESTING_MODELS_DIR}/vulcan -y
                RESULT_VARIABLE EXTRACT_RESULT OUTPUT_QUIET
            )
            if(EXTRACT_RESULT EQUAL 0)
                message(STATUS "  Extracted to ${TESTING_MODELS_DIR}/vulcan")
            endif()
        endif()
    else()
        message(STATUS "Testing models already extracted at ${TESTING_MODELS_DIR}")
    endif()
elseif(EXISTS "${TESTING_ARCHIVE_DIR}")
    message(STATUS "7z not found - testing model archives will be extracted at runtime by ReferenceModelAnalyzer")
endif()

# Also copy any OBJ files placed directly in the source models/ships/ directory
# so that users can drop .obj files there and have them picked up at build time.
set(SRC_MODELS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models/ships")
file(GLOB SRC_OBJ_FILES CONFIGURE_DEPENDS "${SRC_MODELS_DIR}/*.obj")
if(SRC_OBJ_FILES)
    add_custom_command(TARGET atlas_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SRC_MODELS_DIR}"
            "$<TARGET_FILE_DIR:atlas_client>/models/ships"
        COMMENT "Copying source OBJ ship models next to executable"
    )
endif()

# Installation
install(TARGETS atlas_client
    RUNTIME DESTINATION bin
)

install(DIRECTORY shaders/
    DESTINATION shaders
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag" PATTERN "*.glsl"
)

install(DIRECTORY assets/
    DESTINATION assets
)

install(DIRECTORY ui_resources/
    DESTINATION ui_resources
    FILES_MATCHING PATTERN "*.rml" PATTERN "*.rcss"
)

install(DIRECTORY models/ships/
    DESTINATION models/ships
    FILES_MATCHING PATTERN "*.obj"
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
        glm::glm
    )
    
    if(USE_GLEW)
        target_link_libraries(test_asteroid_field GLEW::GLEW)
    endif()
    
    if(WIN32)
        target_link_libraries(test_asteroid_field ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_asteroid_field dl X11)
    elseif(APPLE)
        target_link_libraries(test_asteroid_field dl)
        target_link_libraries(test_asteroid_field "-framework Cocoa -framework IOKit -framework CoreVideo")
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
        glm::glm
    )
    
    if(USE_GLEW)
        target_link_libraries(test_shadow_mapping GLEW::GLEW)
    endif()
    
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
        glm::glm
    )
    
    if(USE_GLEW)
        target_link_libraries(test_deferred_rendering GLEW::GLEW)
    endif()
    
    if(WIN32)
        target_link_libraries(test_deferred_rendering ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_deferred_rendering dl X11)
    elseif(APPLE)
        target_link_libraries(test_deferred_rendering dl)
        target_link_libraries(test_deferred_rendering "-framework Cocoa -framework IOKit -framework CoreVideo")
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
        glm::glm
    )
    if(USE_GLEW)
        target_link_libraries(test_post_processing GLEW::GLEW)
    endif()
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
            glm::glm
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
    if(nlohmann_json_FOUND)
        target_link_libraries(test_network nlohmann_json::nlohmann_json)
    endif()
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
        glm::glm
    )
    if(WIN32)
        target_link_libraries(test_entity_sync ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_entity_sync)
    elseif(APPLE)
        target_link_libraries(test_entity_sync)
    endif()
    
    
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
    if(nlohmann_json_FOUND)
        target_link_libraries(test_server_responses nlohmann_json::nlohmann_json)
    endif()
    if(WIN32)
        target_link_libraries(test_server_responses ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_server_responses)
    elseif(APPLE)
        target_link_libraries(test_server_responses)
    endif()

    # Test: Reference Model Analyzer
    # Analyzes uploaded 3D models from testing/ directory and validates
    # that learned parameters produce valid procedural geometry.
    add_executable(test_model_analyzer
        test_model_analyzer.cpp
        src/rendering/reference_model_analyzer.cpp
        src/rendering/procedural_mesh_ops.cpp
        src/rendering/ship_generation_rules.cpp
    )
    target_link_libraries(test_model_analyzer
        Threads::Threads
        glm::glm
    )
    if(WIN32)
        target_link_libraries(test_model_analyzer ws2_32)
    endif()

    # Test: Atlas UI System (headless — no GPU required)
    add_executable(test_atlas_ui
        test_atlas_ui.cpp
        src/ui/atlas/atlas_renderer.cpp
        src/ui/atlas/atlas_context.cpp
        src/ui/atlas/atlas_widgets.cpp
        src/ui/atlas/atlas_hud.cpp
        src/ui/rml_ui_manager.cpp
        src/ui/context_menu.cpp
    )
    target_include_directories(test_atlas_ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_definitions(test_atlas_ui PRIVATE ATLAS_HEADLESS)
    target_link_libraries(test_atlas_ui
        Threads::Threads
        glm::glm
    )
    if(WIN32)
        target_link_libraries(test_atlas_ui ws2_32)
    endif()

    # Test: Layout Manager (headless — no GPU required)
    add_executable(test_layout_manager
        test_layout_manager.cpp
        src/ui/layout_manager.cpp
    )
    target_include_directories(test_layout_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(test_layout_manager
        Threads::Threads
    )
    if(WIN32)
        target_link_libraries(test_layout_manager ws2_32)
    endif()
endif()

# RmlUi Test (only when RmlUi is enabled)
if(BUILD_TESTS AND RMLUI_AVAILABLE)
    add_executable(test_rmlui
        test_rmlui.cpp
        src/ui/rml_ui_manager.cpp
        src/core/entity.cpp
        ${RMLUI_BACKEND_SOURCES}
    )
    target_compile_definitions(test_rmlui PRIVATE USE_RMLUI)
    target_link_libraries(test_rmlui
        Threads::Threads
        OpenGL::GL
        glfw
        glm::glm
    )
    if(USE_GLEW)
        target_link_libraries(test_rmlui GLEW::GLEW)
    endif()
    if(RmlUi_FOUND)
        target_link_libraries(test_rmlui RmlUi::RmlUi)
    elseif(TARGET rmlui_core)
        target_link_libraries(test_rmlui rmlui_core rmlui_debugger)
    endif()
    if(WIN32)
        target_link_libraries(test_rmlui ws2_32)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_rmlui dl X11)
    elseif(APPLE)
        target_link_libraries(test_rmlui dl)
        target_link_libraries(test_rmlui "-framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
    # Copy UI resources for the test
    add_custom_command(TARGET test_rmlui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/ui_resources
            $<TARGET_FILE_DIR:test_rmlui>/ui_resources
        COMMENT "Copying UI resources for test_rmlui"
    )
    message(STATUS "RmlUi test: test_rmlui will be built")
endif()

message(STATUS "")
message(STATUS "Atlas Client Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenGL: ${OPENGL_VERSION}")
message(STATUS "  Use System Libraries: ${USE_SYSTEM_LIBS}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  RmlUi Enabled: ${RMLUI_AVAILABLE}")
message(STATUS "")
