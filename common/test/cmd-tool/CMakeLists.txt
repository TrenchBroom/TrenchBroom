set(CMD_TOOL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMD_TOOL_SOURCE
        "${CMD_TOOL_SOURCE_DIR}/Main.cpp")
set(CMD_TOOL_SOURCE ${CMD_TOOL_SOURCE})

add_executable(cmd-tool ${CMD_TOOL_SOURCE})
target_include_directories(cmd-tool PRIVATE ${CMD_TOOL_SOURCE_DIR})

set_compiler_config(cmd-tool)
