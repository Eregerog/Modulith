# ImGUI CMakeLists

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_widgets.cpp)
add_library(imgui SHARED ${SOURCE_LIST})
set_target_properties(imgui PROPERTIES DEBUG_POSTFIX "")

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(EXPORT_IMGUI)

add_custom_command(TARGET imgui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:imgui> ${PROJECT_SOURCE_DIR}/build/imgui.dll
        )