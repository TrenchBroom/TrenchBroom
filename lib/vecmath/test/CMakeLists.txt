add_executable(vecmath-test)
target_sources(vecmath-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/run_all.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_bbox.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_bezier_surface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_convex_hull.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_distance.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_intersection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_line.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_mat_ext.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_mat_io.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_mat.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_plane.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_polygon.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_quat.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ray.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_scalar.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_segment.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_vec_ext.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_vec_io.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_vec.cpp"
        )

target_link_libraries(vecmath-test Catch2::Catch2 vecmath)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(vecmath-test PRIVATE -Wall -Wextra -Wconversion -pedantic -Wno-c++98-compat -Wno-global-constructors -Wno-zero-as-null-pointer-constant)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vecmath-test PRIVATE -Wall -Wextra -Wconversion -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(vecmath-test PRIVATE /W3 /EHsc /MP)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()
