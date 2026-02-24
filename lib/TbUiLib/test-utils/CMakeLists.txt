add_library(TbUiTestUtilsLib STATIC)

target_sources(TbUiTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MapDocumentFixture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StringMakers.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbUiTestUtilsLib PRIVATE
  CompilerConfig
  PrecompileQtHeaders
  PrecompileStdHeaders
)
target_link_libraries(TbUiTestUtilsLib PRIVATE
  Catch2::Catch2
  Qt6::Network
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  Qt6::Svg
  Qt6::Widgets
  TbBaseLib
  TbGlTestUtilsLib
  TbMdlLib
  TbMdlTestUtilsLib
  TbUiLib
)
