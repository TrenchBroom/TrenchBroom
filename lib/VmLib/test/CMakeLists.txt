add_executable(VmLibTest)
target_sources(VmLibTest PRIVATE
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

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(VmLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(VmLibTest PRIVATE Catch2::Catch2WithMain VmLib)
