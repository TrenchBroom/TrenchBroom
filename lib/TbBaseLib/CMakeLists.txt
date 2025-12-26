cmake_minimum_required(VERSION 3.12)

add_library(TbBaseLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Color.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorChannel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FileLocation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoggerCache.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LoggingHub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/NotifierConnection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ParserException.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ParserStatus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Preference.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferenceManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PreferenceStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SimpleParserStatus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Uuid.cpp
)

target_include_directories(TbBaseLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbBaseLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbBaseLib PRIVATE stduuid)
target_link_libraries(TbBaseLib PUBLIC KdLib VmLib fmt::fmt-header-only)

add_subdirectory(test)
add_subdirectory(test-utils)
