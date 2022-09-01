# Runtime CMakeLists

add_executable(ModulithRuntime src/Runtime.cpp src/Runtime.h)

target_link_libraries(ModulithRuntime PRIVATE ModulithEngine)

set(OutputDirectory ${PROJECT_SOURCE_DIR}/build/)

add_custom_command(TARGET ModulithRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ModulithRuntime> ${OutputDirectory}/ModulithRuntime.exe
        )