# Tests CMakeLists

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "src/**.cpp" "src/**.h" "extern/**.hpp")

add_executable(ModulithEngineTests ${SOURCE_LIST})

target_include_directories(ModulithEngineTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/)
target_link_libraries(ModulithEngineTests PRIVATE ModulithEngine)