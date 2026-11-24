cmake_minimum_required(VERSION 4.5.0)
# I'm using import std, no apologies.

project(alsa_min
    VERSION 0.0.1
    DESCRIPTION "minimal alsa example"
    LANGUAGES CXX)

include(GNUInstallDirs)

if(PROJECT_IS_TOP_LEVEL)
    # setting export compile commands here globally so that clang-tidy checks are ignored in imported builds
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    include(CTest)
endif()

add_subdirectory(include)
add_subdirectory(test)
