message(STATUS "Fetching cpptrace...")
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace
  GIT_TAG        3db8da80111171c219ab5839905771386bee06b3 # v1.0.4
  SYSTEM
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/patches/cpptrace-strip-msvc-flags.cmake
)

# cpptrace and its bundled dependencies (libdwarf, zstd) build as static
# libraries. zstd warns when BUILD_SHARED_LIBS is ON (enabled globally by
# assimp), so force it off here to keep the configure output clean.
set(_tb_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(cpptrace)
set(BUILD_SHARED_LIBS ${_tb_saved_build_shared_libs})

suppress_dependency_warnings(cpptrace-lib)

if(TARGET dwarf)
  suppress_dependency_warnings(dwarf)
endif()

if(TARGET libzstd_static)
  suppress_dependency_warnings(libzstd_static)
endif()
