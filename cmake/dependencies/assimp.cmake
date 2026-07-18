message(STATUS "Fetching assimp...")
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp
  GIT_TAG        fb375dd8c0a032106a2122815fb18dffe0283721 # v6.0.2
  SYSTEM
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/patches/assimp-strip-msvc-flags.cmake
)

# We only import models, so skip exporters, tests, tools and samples, and don't
# install anything. Disable warnings-as-errors so assimp builds under CMake 4.
set(ASSIMP_NO_EXPORT ON)
# Use the system zlib (assimp's bundled copy is too old to compile against
# recent SDKs), but force assimp's bundled minizip/unzip. Otherwise assimp's
# pkg-config lookup finds Homebrew's minizip, whose unzip.h sits under a
# minizip/ subdir and doesn't match assimp's #include <unzip.h>.
set(ASSIMP_BUILD_MINIZIP ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
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
FetchContent_MakeAvailable(assimp)
set(BUILD_SHARED_LIBS ${_tb_saved_build_shared_libs})
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(assimp)
