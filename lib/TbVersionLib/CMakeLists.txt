add_library(TbVersionLib INTERFACE)

target_sources(TbVersionLib
  INTERFACE FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Create the cmake script for generating the version information
add_custom_target(TbVersionLib_GenerateVersion ${CMAKE_COMMAND}
  -D UTILS="${CMAKE_SOURCE_DIR}/cmake/Utils.cmake"
  -D APP_BUILD_TYPE="${APP_BUILD_TYPE}"
  -D SRC="${CMAKE_CURRENT_SOURCE_DIR}/include/version/Version.h.in"
  -D DST="${CMAKE_CURRENT_BINARY_DIR}/generated/version/Version.h"
  -D GIT_EXECUTABLE="${GIT_EXECUTABLE}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersion.cmake")
add_dependencies(TbVersionLib TbVersionLib_GenerateVersion)
