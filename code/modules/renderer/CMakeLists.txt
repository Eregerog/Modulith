# ======================================================================================================================
# RENDERER MODULE CMAKELISTS
# ======================================================================================================================

# ----------------------------------------------------------------------------------------------------------------------
# TARGET VARIABLES: Configured Automatically, do not change!
# ----------------------------------------------------------------------------------------------------------------------

set(ModuleName Renderer)
set(ModuleDependencies Core)

set(ExecutableDirectory ${PROJECT_SOURCE_DIR}/build/)
set(ModulesDirectory ${ExecutableDirectory}/modules/)
set(SourceDirectory ${CMAKE_CURRENT_SOURCE_DIR})
set(OutputDirectory ${ModulesDirectory}/${ModuleName}/)
set(ModdingOutputDirectory ${OutputDirectory}/modding/)

set(EngineModdingDirectory ${ExecutableDirectory}/modding/)

set(LinkingDependencies ModulithEngine spdlog crossguid yaml-cpp)

# ----------------------------------------------------------------------------------------------------------------------
# BLUEPRINT: Generated Automatically, the same for every module. Do not change!
# ----------------------------------------------------------------------------------------------------------------------

file(GLOB_RECURSE SourceList CONFIGURE_DEPENDS "src/**.cpp" "src/**.h")
add_library(${ModuleName} SHARED ${SourceList})
set_target_properties(${ModuleName} PROPERTIES DEBUG_POSTFIX "")

if (${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    target_precompile_headers(${ModuleName} PRIVATE ${EngineModdingDirectory}/include/PreCompiledHeader.h include/ModulithPreCompiledHeader.h)
endif ()

# needed because the pdb files are still "locked" when recompiling while playing: This allows hotloading
string(TIMESTAMP NOW "%Y-%m-%d %H-%M-%S")
set_target_properties(${ModuleName} PROPERTIES PDB_NAME "${ModuleName} ${NOW}")

target_include_directories(${ModuleName} PUBLIC ${EngineModdingDirectory}/include/)
target_include_directories(${ModuleName} PUBLIC include/)
target_include_directories(${ModuleName} PRIVATE src/)

foreach(toLink IN ITEMS ${LinkingDependencies})
    target_link_libraries(${ModuleName} PRIVATE "${EngineModdingDirectory}/${toLink}.lib")
endforeach()

foreach(dependency IN ITEMS ${ModuleDependencies})
    file(GLOB ModuleLinkingDependencies CONFIGURE_DEPENDS "${ModulesDirectory}/${dependency}/modding/**.lib")
    foreach(toLink IN ITEMS ${ModuleLinkingDependencies})
        target_link_libraries(${ModuleName} PRIVATE ${toLink})
    endforeach()
    target_include_directories(${ModuleName} PRIVATE ${ModulesDirectory}/${dependency}/modding/include)
endforeach()

add_custom_command(TARGET ${ModuleName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ModuleName}> ${OutputDirectory}/${ModuleName}_hotloadable.dll
        )

add_custom_command(TARGET ${ModuleName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:${ModuleName}> ${ModdingOutputDirectory}/${ModuleName}.lib
        )

if(EXISTS ${SourceDirectory}/include/)
    add_custom_command(TARGET ${ModuleName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${SourceDirectory}/include/ ${ModdingOutputDirectory}/include/
            )
endif()

target_compile_definitions(${ModuleName} PUBLIC SPDLOG_COMPILED_LIB)
target_compile_definitions(${ModuleName} PRIVATE EXPORT_$<UPPER_CASE:${ModuleName}>_MODULE)

# ----------------------------------------------------------------------------------------------------------------------
# (Optional) TARGET-SPECIFIC CMAKE:
#       You can paste custom cmake commands below that should apply to this target.
#       It is recommended to use the target variables instead of direct names / paths
# ----------------------------------------------------------------------------------------------------------------------
