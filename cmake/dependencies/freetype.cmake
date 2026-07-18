message(STATUS "Fetching freetype...")
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype
  GIT_TAG        534ad3456055ee1f65ecde3bcf22a656a31514d1 # VER-2-13-3
  SYSTEM
)

# TrenchBroom only rasterizes TrueType fonts, so build a minimal freetype
# without its optional external dependencies. Use CACHE ... FORCE so the values
# reliably override freetype's own option() defaults.
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
# freetype declares an old cmake_minimum_required, which triggers a deprecation
# warning under CMake 4.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(freetype)
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(freetype)
