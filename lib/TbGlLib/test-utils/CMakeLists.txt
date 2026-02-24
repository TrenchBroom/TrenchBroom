add_library(TbGlTestUtilsLib STATIC)

target_sources(TbGlTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbGlTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbGlTestUtilsLib PUBLIC TbGlLib)
