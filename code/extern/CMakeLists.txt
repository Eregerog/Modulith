# Extern CMakeLists

add_subdirectory(glfw)

add_subdirectory(glad)

add_subdirectory(spdlog)
# spdlog and mingw currently do not work together under all circumstances when thread local storage is used, so it disabled here
target_compile_definitions(spdlog PUBLIC SPDLOG_NO_TLS)

add_subdirectory(glm)
# GLM is not (yet) following modern CMake Best Practice
# We create a INTERFACE target (no binaries) to inject the include paths
add_library(glm_header INTERFACE)
target_include_directories(glm_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm/)

add_subdirectory(yaml-cpp)

add_subdirectory(imgui)

add_subdirectory(assimp)

add_subdirectory(crossguid)