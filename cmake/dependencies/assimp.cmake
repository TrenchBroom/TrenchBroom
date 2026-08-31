# Some of assimp's bundled contrib libraries declare a cmake_minimum_required
# below 3.10, which triggers a deprecation warning under CMake 4.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
# assimp's CMakeLists sets the BUILD_SHARED_LIBS cache variable to ON as a
# side effect, which would otherwise leak into every dependency fetched after
# it and switch them from static to shared. Save and restore it so assimp
# (and everything else) keeps building static, matching the previous vcpkg
# (static triplet) behavior.
set(_tb_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
CPMAddPackage(
  URI "gh:assimp/assimp#v6.0.2"
  PATCHES "patches/assimp-strip-msvc-flags.patch"
  OPTIONS
    # We only import models, so skip exporters, tests, tools and samples.
    # Disable warnings-as-errors so assimp builds under CMake 4.
    "ASSIMP_NO_EXPORT ON"
    # Use the system zlib (assimp's bundled copy is too old to compile against
    # recent SDKs), but force assimp's bundled minizip/unzip. Otherwise
    # assimp's pkg-config lookup finds Homebrew's minizip, whose unzip.h sits
    # under a minizip/ subdir and doesn't match assimp's #include <unzip.h>.
    "ASSIMP_BUILD_MINIZIP ON"
    "ASSIMP_BUILD_TESTS OFF"
    "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
    "ASSIMP_BUILD_SAMPLES OFF"
    "ASSIMP_WARNINGS_AS_ERRORS OFF"
    # assimp otherwise appends "/Zi" to CMAKE_CXX_FLAGS_RELEASE for MSVC to
    # build its own PDBs, which conflicts with the debug-info format this
    # project applies to all fetched dependencies (see
    # CMAKE_MSVC_DEBUG_INFORMATION_FORMAT in the top-level CMakeLists.txt),
    # producing a harmless but noisy "cl : Command line warning D9025". We
    # don't install anything from assimp, so we don't need its PDBs either.
    "ASSIMP_INSTALL_PDB OFF"
)
set(BUILD_SHARED_LIBS ${_tb_saved_build_shared_libs})
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(assimp)
apply_sanitizer_options(assimp)
