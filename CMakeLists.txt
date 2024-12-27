cmake_minimum_required(VERSION 3.30)

include(FetchContent)

# LIBS

# # MidiParser

FetchContent_Declare(
  midiparser
  GIT_REPOSITORY https://github.com/somecho/MidiParser.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(midiparser)

# # SDL2

find_package(SDL2 REQUIRED)


# PROJECT CONFIGURATIONS

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(can)

add_executable(can src/main.cpp)

target_compile_features(can PRIVATE cxx_std_23)

target_link_libraries(can PRIVATE SDL2 MidiParser)
