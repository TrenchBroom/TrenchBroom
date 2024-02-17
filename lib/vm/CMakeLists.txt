cmake_minimum_required(VERSION 3.12)

project(vm VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(VM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vm INTERFACE)

target_sources(vm INTERFACE
    "${VM_INCLUDE_DIR}/vm/abstract_line.h"
    "${VM_INCLUDE_DIR}/vm/approx.h"
    "${VM_INCLUDE_DIR}/vm/bbox_io.h"
    "${VM_INCLUDE_DIR}/vm/bbox.h"
    "${VM_INCLUDE_DIR}/vm/bezier_surface.h"
    "${VM_INCLUDE_DIR}/vm/constants.h"
    "${VM_INCLUDE_DIR}/vm/constexpr_util.h"
    "${VM_INCLUDE_DIR}/vm/convex_hull.h"
    "${VM_INCLUDE_DIR}/vm/distance.h"
    "${VM_INCLUDE_DIR}/vm/forward.h"
    "${VM_INCLUDE_DIR}/vm/glsh.h"
    "${VM_INCLUDE_DIR}/vm/intersection.h"
    "${VM_INCLUDE_DIR}/vm/line_io.h"
    "${VM_INCLUDE_DIR}/vm/line.h"
    "${VM_INCLUDE_DIR}/vm/mat_ext.h"
    "${VM_INCLUDE_DIR}/vm/mat_io.h"
    "${VM_INCLUDE_DIR}/vm/mat.h"
    "${VM_INCLUDE_DIR}/vm/plane_io.h"
    "${VM_INCLUDE_DIR}/vm/plane.h"
    "${VM_INCLUDE_DIR}/vm/polygon.h"
    "${VM_INCLUDE_DIR}/vm/quat.h"
    "${VM_INCLUDE_DIR}/vm/ray_io.h"
    "${VM_INCLUDE_DIR}/vm/ray.h"
    "${VM_INCLUDE_DIR}/vm/scalar.h"
    "${VM_INCLUDE_DIR}/vm/segment.h"
    "${VM_INCLUDE_DIR}/vm/util.h"
    "${VM_INCLUDE_DIR}/vm/vec_ext.h"
    "${VM_INCLUDE_DIR}/vm/vec_io.h"
    "${VM_INCLUDE_DIR}/vm/vec.h"
)

target_include_directories(vm INTERFACE
        $<BUILD_INTERFACE:${VM_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:vm/include/vm>)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(vm INTERFACE -Wall -Wextra -pedantic -Wshadow-all -Wno-c++98-compat -Wno-float-equal)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm INTERFACE -Wall -Wextra -pedantic -Wshadow=local)
elseif(MSVC EQUAL 1)
    target_compile_options(vm INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(vm INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

add_subdirectory(test)
