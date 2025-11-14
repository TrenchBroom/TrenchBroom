cmake_minimum_required(VERSION 3.12)

project(vm VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(LIB_VM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(lib-vm INTERFACE)
target_include_directories(lib-vm INTERFACE
        $<BUILD_INTERFACE:${LIB_VM_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:vm/include/vm>)

target_sources(lib-vm INTERFACE
    "${LIB_VM_INCLUDE_DIR}/vm/abstract_line.h"
    "${LIB_VM_INCLUDE_DIR}/vm/approx.h"
    "${LIB_VM_INCLUDE_DIR}/vm/bbox_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/bbox.h"
    "${LIB_VM_INCLUDE_DIR}/vm/bezier_surface.h"
    "${LIB_VM_INCLUDE_DIR}/vm/constants.h"
    "${LIB_VM_INCLUDE_DIR}/vm/constexpr_util.h"
    "${LIB_VM_INCLUDE_DIR}/vm/convex_hull.h"
    "${LIB_VM_INCLUDE_DIR}/vm/distance.h"
    "${LIB_VM_INCLUDE_DIR}/vm/from_chars.h"
    "${LIB_VM_INCLUDE_DIR}/vm/intersection.h"
    "${LIB_VM_INCLUDE_DIR}/vm/line_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/line.h"
    "${LIB_VM_INCLUDE_DIR}/vm/mat_ext.h"
    "${LIB_VM_INCLUDE_DIR}/vm/mat_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/mat.h"
    "${LIB_VM_INCLUDE_DIR}/vm/plane_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/plane.h"
    "${LIB_VM_INCLUDE_DIR}/vm/polygon.h"
    "${LIB_VM_INCLUDE_DIR}/vm/quat.h"
    "${LIB_VM_INCLUDE_DIR}/vm/ray_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/ray.h"
    "${LIB_VM_INCLUDE_DIR}/vm/scalar.h"
    "${LIB_VM_INCLUDE_DIR}/vm/segment.h"
    "${LIB_VM_INCLUDE_DIR}/vm/util.h"
    "${LIB_VM_INCLUDE_DIR}/vm/vec_ext.h"
    "${LIB_VM_INCLUDE_DIR}/vm/vec_io.h"
    "${LIB_VM_INCLUDE_DIR}/vm/vec.h"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(lib-vm INTERFACE -Wall -Wextra -pedantic -Wshadow-all -Wno-c++98-compat -Wno-c++20-compat -Wno-float-equal)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(lib-vm INTERFACE -Wall -Wextra -pedantic -Wshadow=local)
elseif(MSVC EQUAL 1)
    target_compile_options(lib-vm INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(lib-vm INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
target_link_libraries(lib-vm INTERFACE FastFloat::fast_float)
endif()

add_subdirectory(test)
