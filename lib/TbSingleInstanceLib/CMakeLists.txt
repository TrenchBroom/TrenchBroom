add_library(TbSingleInstanceLib STATIC)

target_sources(TbSingleInstanceLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SingleInstance.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/singleinstance/SingleInstance.h
)

target_link_libraries(TbSingleInstanceLib
  PRIVATE
    CompilerConfig
    Qt6::Network
  PUBLIC
    Qt6::Core
)

set_target_properties(TbSingleInstanceLib PROPERTIES AUTOMOC TRUE)

add_subdirectory(test)
