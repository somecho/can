cmake_minimum_required(VERSION 3.30)

include(FetchContent)

# LIBS

# # MidiParser

FetchContent_Declare(
  midiparser
  GIT_REPOSITORY https://github.com/somecho/MidiParser.git
  GIT_TAG        0.2.6
)

# # SDL2

set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC ON CACHE INTERNAL "")

FetchContent_Declare(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.30.10
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(midiparser sdl)

# PROJECT CONFIGURATIONS

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_compile_options("$<$<CONFIG:Debug>:-g;-Wall;-Wpedantic;-Wconversion>")
add_compile_options("$<$<CONFIG:Release>:-O3;-DNDEBUG;-s;-Wall;-Wpedantic>")

project(can VERSION 0.1.1 LANGUAGES CXX)

add_library(canviewers
  src/can/MidiViewer.cpp
  src/can/helper.cpp
)

target_link_libraries(canviewers PUBLIC SDL2::SDL2 MidiParser)
set_property(TARGET canviewers PROPERTY CXX_STANDARD 23)
set_property(TARGET canviewers PROPERTY CXX_STANDARD_REQUIRED TRUE)

add_executable(can src/can.cpp)
target_compile_features(can PRIVATE cxx_std_23)
target_link_libraries(can PRIVATE canviewers)
