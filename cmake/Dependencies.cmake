include(FetchContent)

include(cmake/dependencies/CompilerConfig.cmake)

include(cmake/dependencies/assimp.cmake)
include(cmake/dependencies/catch2.cmake)
include(cmake/dependencies/cpptrace.cmake)
include(cmake/dependencies/ctre.cmake)
include(cmake/dependencies/fmt.cmake)
include(cmake/dependencies/freeimage.cmake)
include(cmake/dependencies/freetype.cmake)
include(cmake/dependencies/miniz.cmake)
include(cmake/dependencies/stduuid.cmake)
include(cmake/dependencies/tinyxml2.cmake)

# Find Qt and OpenGL
find_package(OpenGL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets Network Svg Test REQUIRED)

if(DEFINED Qt6_DIR)
  get_filename_component(TB_QT_LIBRARY_DIR "${Qt6_DIR}/../.." ABSOLUTE)
  message(STATUS "Found Qt6: ${TB_QT_LIBRARY_DIR}")
endif()

# Find threads lib, needed to work around a gtest bug, see: https://stackoverflow.com/questions/21116622/undefined-reference-to-pthread-key-create-linker-error
# The googletest target links to this
find_package(Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
include(cmake/dependencies/fast-float.cmake)
endif()
