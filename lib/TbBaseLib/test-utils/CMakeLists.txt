add_library(TbBaseTestUtilsLib STATIC)

target_sources(TbBaseTestUtilsLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Matchers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestLogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestParserStatus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TestPreferenceStore.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbBaseTestUtilsLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2
  PUBLIC
    TbBaseLib
)

add_subdirectory(test)
