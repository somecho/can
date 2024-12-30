cmake_minimum_required(VERSION 3.30)

project(can VERSION 0.1.3 LANGUAGES CXX)

include(FetchContent)

# LIBS

# # MidiParser

FetchContent_Declare(
  midiparser
  GIT_REPOSITORY https://github.com/somecho/MidiParser.git
  GIT_TAG        0.2.6
)

# # SDL3

set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC ON CACHE INTERNAL "")

FetchContent_Declare(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG c030e6f782d8641e7bd8c1dc2d58788cab4f2e4d
)

FetchContent_MakeAvailable(midiparser sdl)

# # FREETYPE

FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype
  GIT_TAG VER-2-13-3
)

FetchContent_MakeAvailable(freetype)

if(NOT EXISTS ${freetype_BINARY_DIR}/libfreetype.a)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -B ${freetype_BINARY_DIR}
                WORKING_DIRECTORY ${freetype_SOURCE_DIR})
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build ${freetype_BINARY_DIR}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

list(APPEND CMAKE_PREFIX_PATH ${freetype_BINARY_DIR})
set(FREETYPE_INCLUDE_DIRS ${freetype_SOURCE_DIR}/include)

# # SDL_TTF

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

FetchContent_Declare(
  sdlttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG a71b5fa4033b0124672e0e4f63df0c0e98d9c648
  GIT_SHALLOW TRUE
  GIT_SUBMODULES ""
)

FetchContent_MakeAvailable(sdlttf)

# PROJECT CONFIGURATIONS

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_compile_options("$<$<CONFIG:Debug>:-g;-Wall;-Wpedantic;-Wconversion>")
add_compile_options("$<$<CONFIG:Release>:-O3;-DNDEBUG;-s;-Wall;-Wpedantic>")

add_library(canviewers
  src/can/MidiViewer.cpp
  src/can/helper.cpp
)

target_link_libraries(canviewers PUBLIC SDL3::SDL3 MidiParser)
set_property(TARGET canviewers PROPERTY CXX_STANDARD 23)
set_property(TARGET canviewers PROPERTY CXX_STANDARD_REQUIRED TRUE)

add_executable(can src/can.cpp)
target_compile_features(can PRIVATE cxx_std_23)
target_link_libraries(can PRIVATE canviewers SDL3_ttf::SDL3_ttf)
target_include_directories(can PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fonts)
target_compile_definitions(can PRIVATE "$<$<CONFIG:Debug>:DEBUG>")
