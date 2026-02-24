add_library(TbMdlTestUtilsLib STATIC)

target_sources(TbMdlTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityDefinitionTestUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MapFixture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Matchers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StringMakers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbMdlTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbMdlTestUtilsLib PUBLIC TbBaseTestUtilsLib TbGlTestUtilsLib TbMdlLib fmt::fmt-header-only)

add_subdirectory(test)
