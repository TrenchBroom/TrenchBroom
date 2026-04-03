add_library(TbPreferencesLib STATIC)

target_sources(TbPreferencesLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbPreferencesLib
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
  PUBLIC
    Qt6::Gui
    TbBaseLib
)
