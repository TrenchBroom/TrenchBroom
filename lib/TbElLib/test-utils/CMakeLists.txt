add_library(TbElTestUtilsLib STATIC)

target_sources(TbElTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbElTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbElTestUtilsLib PRIVATE TbElLib)
