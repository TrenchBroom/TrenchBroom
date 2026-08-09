add_library(TbAppTestUtilsLib STATIC)

target_sources(TbAppTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MapDocumentFixture.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbAppTestUtilsLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
  PUBLIC
    TbAppLib
    TbMdlTestUtilsLib
)
