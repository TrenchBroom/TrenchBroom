message(STATUS "Fetching miniz...")
FetchContent_Declare(
  miniz
  GIT_REPOSITORY https://github.com/richgel999/miniz
  GIT_TAG        174573d60290f447c13a2b1b3405de2b96e27d6c # 3.1.0
  SYSTEM
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/patches/miniz-strip-msvc-flags.cmake
)

# miniz's BUILD_EXAMPLES/BUILD_TESTS/INSTALL_PROJECT options already default to
# off when miniz is built as a subproject, so no configuration is needed.
FetchContent_MakeAvailable(miniz)

suppress_dependency_warnings(miniz)
