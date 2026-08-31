# cpptrace and its bundled dependencies (libdwarf, zstd) build as static
# libraries. zstd warns when BUILD_SHARED_LIBS is ON (enabled globally by
# assimp), so force it off here to keep the configure output clean.
set(_tb_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
CPMAddPackage(
  URI "gh:jeremy-rifkin/cpptrace#v1.0.4"
  PATCHES "patches/cpptrace-strip-msvc-flags.patch"
)
set(BUILD_SHARED_LIBS ${_tb_saved_build_shared_libs})

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
