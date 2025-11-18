cmake_minimum_required(VERSION 3.12)

project(VmLib VERSION 0.1.0 LANGUAGES CXX)

set(VM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(VmLib INTERFACE)
target_include_directories(VmLib INTERFACE
        $<BUILD_INTERFACE:${VM_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:vm/include/vm>)

target_sources(VmLib INTERFACE
    "${VM_INCLUDE_DIR}/vm/abstract_line.h"
    "${VM_INCLUDE_DIR}/vm/approx.h"
    "${VM_INCLUDE_DIR}/vm/bbox_io.h"
    "${VM_INCLUDE_DIR}/vm/bbox.h"
    "${VM_INCLUDE_DIR}/vm/bezier_surface.h"
    "${VM_INCLUDE_DIR}/vm/constants.h"
    "${VM_INCLUDE_DIR}/vm/constexpr_util.h"
    "${VM_INCLUDE_DIR}/vm/convex_hull.h"
    "${VM_INCLUDE_DIR}/vm/distance.h"
    "${VM_INCLUDE_DIR}/vm/from_chars.h"
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

target_link_libraries(VmLib INTERFACE CompilerConfig)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
target_link_libraries(VmLib INTERFACE FastFloat::fast_float)
endif()

add_subdirectory(test)
