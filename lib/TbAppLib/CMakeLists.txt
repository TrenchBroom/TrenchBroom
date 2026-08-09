add_library(TbAppLib STATIC)

target_sources(TbAppLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Placeholder.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbAppLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
  PUBLIC
    KdLib
    TbBaseLib
    TbMdlLib
    TbPreferencesLib
    TbRenderLib
    VmLib
)
