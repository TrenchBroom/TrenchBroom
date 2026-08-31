# miniz's BUILD_EXAMPLES/BUILD_TESTS options already default to off when miniz
# is built as a subproject, so no configuration is needed.
CPMAddPackage(
  URI "gh:richgel999/miniz#3.1.0"
  PATCHES "patches/miniz-strip-msvc-flags.patch"
)

suppress_dependency_warnings(miniz)
apply_sanitizer_options(miniz)
