cmake_minimum_required(VERSION 3.13)
project(FFmpegPlayer)

set(CMAKE_CXX_STANDARD 14)

# ADD SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(FFmpegPlayer src/main.cpp)

target_link_libraries(FFmpegPlayer
        ${SDL2_LIBRARIES})