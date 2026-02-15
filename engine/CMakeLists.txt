add_library(AtlasEngine STATIC
    core/Engine.cpp
    core/Logger.cpp
    ecs/ECS.cpp
    graphvm/GraphVM.cpp
    graphvm/GraphCompiler.cpp
    assets/AssetRegistry.cpp
    assets/AssetBinary.cpp
    net/NetContext.cpp
    sim/TickScheduler.cpp
    world/CubeSphereLayout.cpp
    world/VoxelGridLayout.cpp
)

target_include_directories(AtlasEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
