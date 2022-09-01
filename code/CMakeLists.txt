# Root CMakeLists
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(/MP)

project(Modulith
        VERSION 1.0.0
        DESCRIPTION "A modular game engine for component-driven development and modding"
        LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/cotire")
include(cotire)

find_package(Doxygen)
if(Doxygen_FOUND)
    add_subdirectory(docs)
else()
    message(STATUS "Doxygen not found, not building docs")
endif()


add_subdirectory(core)
add_subdirectory(modules)
add_subdirectory(extern)
