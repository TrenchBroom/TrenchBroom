cmake_minimum_required(VERSION 3.12)

project(vecmath VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(VECMATH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vecmath INTERFACE)

target_sources(vecmath INTERFACE
    "${VECMATH_INCLUDE_DIR}/vecmath/abstract_line.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/approx.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/bbox_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/bbox.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/bezier_surface.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/constants.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/constexpr_util.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/convex_hull.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/distance.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/forward.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/glsh.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/intersection.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/line_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/line.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/mat_ext.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/mat_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/mat.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/plane_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/plane.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/polygon.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/quat.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/ray_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/ray.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/scalar.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/segment.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/util.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/vec_ext.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/vec_io.h"
    "${VECMATH_INCLUDE_DIR}/vecmath/vec.h"
)

target_include_directories(vecmath INTERFACE
        $<BUILD_INTERFACE:${VECMATH_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:vecmath/include/vecmath>)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(vecmath INTERFACE -Wall -Wextra -pedantic -Wshadow-all -Wno-c++98-compat -Wno-float-equal)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vecmath INTERFACE -Wall -Wextra -pedantic -Wshadow=local)
elseif(MSVC EQUAL 1)
    target_compile_options(vecmath INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(vecmath INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

add_subdirectory(test)
