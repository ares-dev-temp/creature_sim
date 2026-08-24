cmake_minimum_required(VERSION 3.20)
project(creature_sim)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 REQUIRED)

add_executable(creature_sim
    main.cpp
)

target_link_libraries(creature_sim PRIVATE glfw)
