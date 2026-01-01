add_executable(CmdTool)

target_sources(CmdTool PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
)

target_link_libraries(CmdTool PRIVATE CompilerConfig)
