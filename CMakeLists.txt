# EVE OFFLINE - Root CMakeLists.txt
# This creates a unified Visual Studio solution for both C++ client and server

cmake_minimum_required(VERSION 3.15)
project(EVEOffline VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_CLIENT "Build C++ OpenGL client" ON)
option(BUILD_SERVER "Build C++ dedicated server" ON)

# Add subdirectories
if(BUILD_CLIENT)
    message(STATUS "Adding C++ Client to solution...")
    add_subdirectory(cpp_client)
endif()

if(BUILD_SERVER)
    message(STATUS "Adding C++ Server to solution...")
    add_subdirectory(cpp_server)
endif()

message(STATUS "")
message(STATUS "EVE OFFLINE Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Client: ${BUILD_CLIENT}")
message(STATUS "  Build Server: ${BUILD_SERVER}")
message(STATUS "")
message(STATUS "After generation, open the solution file in Visual Studio:")
message(STATUS "  build_vs/EVEOffline.sln")
message(STATUS "")
