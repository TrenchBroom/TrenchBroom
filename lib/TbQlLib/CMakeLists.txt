add_library(TbQlLib STATIC)

target_sources(TbQlLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LayerNodeBinding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NodeQueryValues.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WorldNodeBinding.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbQlLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
  PUBLIC
    fmt::fmt-header-only
    KdLib
    TbBaseLib
    TbElLib
    TbMdlLib
  )

add_subdirectory(test)
