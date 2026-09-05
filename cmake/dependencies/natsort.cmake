# natsort ships no CMake build and no releases, so we compile strnatcmp.c from a
# pinned commit.
CPMAddPackage(
  URI "gh:sourcefrog/natsort#cdd8df9602e727482ae5e051cff74b7ec7ffa07a"
  DOWNLOAD_ONLY YES
)

# assimp turns BUILD_SHARED_LIBS on globally, so say STATIC explicitly.
add_library(natsort STATIC "${natsort_SOURCE_DIR}/strnatcmp.c")
target_include_directories(natsort SYSTEM PUBLIC "${natsort_SOURCE_DIR}")
# strnatcmp.c uses `static inline`, which MSVC only accepts in C11 mode.
target_compile_features(natsort PRIVATE c_std_11)

suppress_dependency_warnings(natsort)
apply_sanitizer_options(natsort)
