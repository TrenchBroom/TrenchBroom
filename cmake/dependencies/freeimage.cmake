message(STATUS "Fetching FreeImage...")
FetchContent_Declare(
  FreeImage
  GIT_REPOSITORY https://github.com/danoli3/FreeImage
  GIT_TAG        625117b48e18a82f3f68e9be3514ae584ebfcf7b # 3.19.11
  SYSTEM
)

# This fork builds all of FreeImage's bundled codecs (zlib, libpng, libjpeg,
# libtiff, webp), so no external image libraries are needed.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(FreeImage)
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(FreeImage)

# The fork's FreeImage target does not expose its public header directory.
target_include_directories(FreeImage SYSTEM INTERFACE $<BUILD_INTERFACE:${freeimage_SOURCE_DIR}/Source>)

# FreeImage's own CMakeLists defines FREEIMAGE_LIB for itself via
# add_definitions() when built static, which is directory-scoped and doesn't
# reach consumers elsewhere in the tree. Without it, FreeImage.h declares the
# API as __declspec(dllimport) for consumers, which fails to link against a
# static library. Propagate it as a usage requirement instead.
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(FreeImage INTERFACE FREEIMAGE_LIB)
endif()
