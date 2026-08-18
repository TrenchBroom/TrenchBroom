add_library(TbVersionLib INTERFACE)

target_sources(TbVersionLib
  INTERFACE FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/generated
)

set(TbVersionLib_GENERATED_HEADER
  ${CMAKE_CURRENT_BINARY_DIR}/generated/version/Version.h)

# A custom_target's COMMAND always reruns, every build, with no way to opt out -- that's
# exactly what's wanted here, since the git describe output can change without any
# tracked input file changing. GenerateVersion.cmake only touches
# TbVersionLib_GENERATED_HEADER's mtime if its contents actually changed, and declaring
# it as a BYPRODUCT (rather than leaving it untracked, as a plain custom_target with no
# BYPRODUCTS did before) gives Ninja a real, restat-tracked graph edge for it: consumers
# are correctly ordered after this command, but only actually rebuilt when the header's
# mtime genuinely changes, not on every rerun of the always-dirty target.
add_custom_target(TbVersionLib_GenerateVersion
  COMMAND ${CMAKE_COMMAND}
    -D UTILS=${CMAKE_SOURCE_DIR}/cmake/Utils.cmake
    -D APP_BUILD_TYPE=${APP_BUILD_TYPE}
    -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/include/version/Version.h.in
    -D DST=${TbVersionLib_GENERATED_HEADER}
    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersion.cmake
  BYPRODUCTS "${TbVersionLib_GENERATED_HEADER}"
  COMMENT "Generating version information"
  VERBATIM)
add_dependencies(TbVersionLib TbVersionLib_GenerateVersion)
