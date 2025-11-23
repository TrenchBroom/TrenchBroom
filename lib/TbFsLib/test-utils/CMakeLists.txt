add_library(TbFsTestUtilsLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TestEnvironment.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp
)

target_include_directories(TbFsTestUtilsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbFsTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbFsTestUtilsLib PUBLIC TbFsLib fmt::fmt-header-only)

add_subdirectory(test)
