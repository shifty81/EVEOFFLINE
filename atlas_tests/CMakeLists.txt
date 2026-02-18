add_executable(AtlasTests
    main.cpp
    test_graphvm.cpp
    test_ecs.cpp
    test_components.cpp
    test_asset.cpp
    test_net.cpp
    test_net_queue.cpp
    test_world.cpp
    test_compiler.cpp
    test_engine.cpp
    test_console.cpp
    test_hud_panels.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/ConsolePanel.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_renderer.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_context.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_widgets.cpp
    ${CMAKE_SOURCE_DIR}/cpp_client/src/ui/atlas/atlas_hud.cpp
)

target_compile_definitions(AtlasTests PRIVATE ATLAS_HEADLESS=1)
target_link_libraries(AtlasTests AtlasEngine)
target_include_directories(AtlasTests PRIVATE
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/cpp_client/include
)

add_test(NAME AtlasTests COMMAND AtlasTests)
