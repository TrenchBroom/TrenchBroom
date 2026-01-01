add_library(TbUiTestUtilsLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MapDocumentFixture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/StringMakers.cpp
)

target_include_directories(TbUiTestUtilsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
  TbMdlLib
  TbMdlTestUtilsLib
  TbUiLib
)
