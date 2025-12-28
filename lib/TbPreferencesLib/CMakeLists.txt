cmake_minimum_required(VERSION 3.12)

add_library(TbPreferencesLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences.cpp
)

target_include_directories(TbPreferencesLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbPreferencesLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbPreferencesLib PUBLIC TbBaseLib Qt6::Gui)
