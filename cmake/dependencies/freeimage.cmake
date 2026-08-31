# This fork builds all of FreeImage's bundled codecs (zlib, libpng, libjpeg,
# libtiff, webp), so no external image libraries are needed.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
CPMAddPackage("gh:danoli3/FreeImage#3.19.11")
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(FreeImage)
apply_sanitizer_options(FreeImage)

# The fork's FreeImage target does not expose its public header directory.
target_include_directories(FreeImage SYSTEM INTERFACE $<BUILD_INTERFACE:${FreeImage_SOURCE_DIR}/Source>)

# FreeImage's own CMakeLists defines FREEIMAGE_LIB for itself via
# add_definitions() when built static, which is directory-scoped and doesn't
# reach consumers elsewhere in the tree. Without it, FreeImage.h declares the
# API as __declspec(dllimport) for consumers, which fails to link against a
# static library. Propagate it as a usage requirement instead.
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(FreeImage INTERFACE FREEIMAGE_LIB)
endif()
