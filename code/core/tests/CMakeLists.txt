# Tests CMakeLists

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "src/**.cpp" "src/**.h")

add_executable(ModulithEngineTests ${SOURCE_LIST})

target_include_directories(ModulithEngineTests PRIVATE ${CMAKE_SOURCE_DIR}/extern/catch/)
target_link_libraries(ModulithEngineTests PRIVATE ModulithEngine)

add_custom_command(TARGET ModulithEngineTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ModulithEngine> ${CMAKE_CURRENT_BINARY_DIR}/ModulithEngine.dll
)