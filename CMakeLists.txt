cmake_minimum_required(VERSION 4.5.0)

project(stompd
    VERSION 0.0.1
    LANGUAGES CXX)

option(STOMPD_BUILD_ENGINE "Build the audio engine" ON)
option(STOMPD_BUILD_UI "Build the Qt UI" ON)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(STOMPD_BUILD_ENGINE)
    add_subdirectory(engine)
endif()
if(STOMPD_BUILD_UI)
    add_subdirectory(ui)
endif()
