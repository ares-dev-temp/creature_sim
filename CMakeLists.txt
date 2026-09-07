cmake_minimum_required(VERSION 3.20)
project(creature_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure we're building for Apple Silicon
set(CMAKE_OSX_ARCHITECTURES arm64)

# Build Jolt
set(USE_STATIC_MSVC_RUNTIME OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/JoltPhysics/Build
    ${CMAKE_BINARY_DIR}/Jolt
)

# GLAD
add_library(glad
    glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

find_package(glfw3 REQUIRED)

add_executable(creature_sim
    rendering/camera.cpp
    rendering/mesh.cpp
    rendering/shader.cpp
    rendering/texture.cpp
    entities/entity.cpp
    main.cpp
)

# Include directories
target_include_directories(creature_sim PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/JoltPhysics
)

target_link_libraries(creature_sim PRIVATE
    glfw
    Jolt
    glad
)

