cmake_minimum_required(VERSION 3.13)
project(FFmpegPlayer)

set(CMAKE_CXX_STANDARD 14)

# ADD OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Modules)

# ADD SDL
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(FFmpegPlayer src/main.cpp)

find_package(FFMPEG REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})

target_link_libraries(FFmpegPlayer
        ${OpenCV_LIBS}
        ${SDL_LIBRARY}
        ${FFMPEG_LIBRARIES})