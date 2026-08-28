add_executable(tbctl)
EMBED_UTF8_MANIFEST(tbctl)

target_sources(tbctl
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
)

target_link_libraries(tbctl
  PRIVATE
    CompilerConfig
    Qt6::Core
    Qt6::Network
)
