# Engine CMakeLists

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "src/**.cpp" "src/**.h" "../../extern/stb_image/stb_image.cpp")
file(GLOB_RECURSE PUBLIC_HEADER_LIST CONFIGURE_DEPENDS "include/**.h")

add_library(ModulithEngine SHARED ${SOURCE_LIST} ${PUBLIC_HEADER_LIST})

# Engine PB

set(OutputDirectory ${PROJECT_SOURCE_DIR}/build/)
set(ModdingOutputDirectory ${OutputDirectory}/modding/)
set_target_properties(ModulithEngine PROPERTIES DEBUG_POSTFIX "")

# Engine Headers + Libraries

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:ModulithEngine> ${ModdingOutputDirectory}/ModulithEngine.lib
        )

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${ModdingOutputDirectory}/include/
        )

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ModulithEngine> ${OutputDirectory}/ModulithEngine.dll
        )

# glm Headers

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/glm/glm ${ModdingOutputDirectory}/include/glm
        )

# spdlog Headers + Library

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/spdlog/include/ ${ModdingOutputDirectory}/include/
        )

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:spdlog::spdlog> ${ModdingOutputDirectory}/spdlog.lib
        )

# crossguid Headers + Library

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/crossguid/include/ ${ModdingOutputDirectory}/include/
        )

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:crossguid> ${ModdingOutputDirectory}/crossguid.lib
        )

# yaml-cpp Library
add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:yaml-cpp> ${ModdingOutputDirectory}/yaml-cpp.lib
        )

add_custom_command(TARGET ModulithEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/yaml-cpp/include/ ${ModdingOutputDirectory}/include/
        )



if (${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    target_precompile_headers(ModulithEngine PUBLIC include/PreCompiledHeader.h)
else ()
    cotire(ModulithEngine)
endif ()

target_compile_definitions(ModulithEngine PRIVATE EXPORT_ENGINE)

target_link_libraries(ModulithEngine PUBLIC glm_header spdlog::spdlog crossguid yaml-cpp)

target_include_directories(ModulithEngine PUBLIC include/)
target_include_directories(ModulithEngine PRIVATE src/)
target_include_directories(ModulithEngine PRIVATE platform/)
target_include_directories(ModulithEngine PRIVATE ../../extern/stb_image/)