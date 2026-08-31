# freetype declares an old cmake_minimum_required, which triggers a deprecation
# warning under CMake 4.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
CPMAddPackage(
  URI "gh:freetype/freetype#VER-2-13-3"
  OPTIONS
    # TrenchBroom only rasterizes TrueType fonts, so build a minimal freetype
    # without its optional external dependencies.
    "FT_DISABLE_ZLIB ON"
    "FT_DISABLE_BZIP2 ON"
    "FT_DISABLE_PNG ON"
    "FT_DISABLE_HARFBUZZ ON"
    "FT_DISABLE_BROTLI ON"
)
unset(CMAKE_POLICY_VERSION_MINIMUM)

suppress_dependency_warnings(freetype)
apply_sanitizer_options(freetype)
