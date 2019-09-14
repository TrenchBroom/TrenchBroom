set(VECMATH_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(VECMATH_TEST_TEST_SOURCE
        "${VECMATH_TEST_SOURCE_DIR}/bbox_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/convex_hull_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/distance_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/intersection_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/line_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/mat_ext_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/mat_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/plane_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/polygon_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/quat_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/ray_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/scalar_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/segment_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/vec_test.cpp"
        "${VECMATH_TEST_SOURCE_DIR}/run_all.cpp"
        )

add_executable(vecmath-test ${VECMATH_TEST_TEST_SOURCE})
target_link_libraries(vecmath-test gtest vecmath)
