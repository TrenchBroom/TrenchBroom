cmake_minimum_required(VERSION 3.12)

add_library(TbPreferencesLib STATIC)

target_sources(TbPreferencesLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbPreferencesLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbPreferencesLib PUBLIC TbBaseLib Qt6::Gui)
