# cpptrace and its bundled dependencies (libdwarf, zstd) build as static
# libraries. zstd warns when BUILD_SHARED_LIBS is ON (enabled globally by
# assimp), so force it off here to keep the configure output clean.
set(_tb_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

# cpptrace fetches zstd and libdwarf itself via FetchContent, which bypasses
# CPM's source cache and downloads them on every fresh configure. Download them
# through CPM instead and point FetchContent at the cached sources; cpptrace
# still adds and configures them as before. Keep the versions in sync with
# CPPTRACE_ZSTD_URL and CPPTRACE_LIBDWARF_TAG in cpptrace's
# cmake/OptionVariables.cmake when updating cpptrace.
CPMAddPackage(
  NAME cpptrace-zstd
  URL "https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  DOWNLOAD_ONLY YES
)
CPMAddPackage(
  NAME cpptrace-libdwarf
  GIT_REPOSITORY "https://github.com/jeremy-rifkin/libdwarf-lite.git"
  GIT_TAG 5dfb2cd2aacf2bf473e5bfea79e41289f88b3a5f # v2.1.0
  DOWNLOAD_ONLY YES
)
set(FETCHCONTENT_SOURCE_DIR_ZSTD ${cpptrace-zstd_SOURCE_DIR})
set(FETCHCONTENT_SOURCE_DIR_LIBDWARF ${cpptrace-libdwarf_SOURCE_DIR})

CPMAddPackage(
  URI "gh:jeremy-rifkin/cpptrace#v1.0.4"
  PATCHES "patches/cpptrace-strip-msvc-flags.patch"
)
set(BUILD_SHARED_LIBS ${_tb_saved_build_shared_libs})
unset(FETCHCONTENT_SOURCE_DIR_ZSTD)
unset(FETCHCONTENT_SOURCE_DIR_LIBDWARF)

suppress_dependency_warnings(cpptrace-lib)
apply_sanitizer_options(cpptrace-lib)

if(TARGET dwarf)
  suppress_dependency_warnings(dwarf)
  apply_sanitizer_options(dwarf)
endif()

if(TARGET libzstd_static)
  suppress_dependency_warnings(libzstd_static)
  apply_sanitizer_options(libzstd_static)
endif()
