add_library(TbFsTestUtilsLib STATIC)

target_sources(TbFsTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestEnvironment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbFsTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbFsTestUtilsLib PUBLIC TbFsLib fmt::fmt-header-only)

add_subdirectory(test)
