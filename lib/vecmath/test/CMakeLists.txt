add_executable(vecmath-test)
target_sources(vecmath-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/bbox_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/bezier_surface_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/convex_hull_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/distance_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/intersection_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/line_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mat_ext_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mat_io_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mat_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/plane_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/polygon_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/quat_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ray_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/segment_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vec_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vec_ext_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vec_io_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/run_all.cpp"
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
