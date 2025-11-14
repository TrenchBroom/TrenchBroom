set(LIB_EL_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_EL_TEST_SOURCE
        "${LIB_EL_TEST_DIR}/tst_EL.cpp"
        "${LIB_EL_TEST_DIR}/tst_ELParser.cpp"
        "${LIB_EL_TEST_DIR}/tst_Expression.cpp"
        "${LIB_EL_TEST_DIR}/tst_Interpolate.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

add_subdirectory(utils)

add_executable(lib-el-test ${LIB_EL_TEST_SOURCE})
target_include_directories(lib-el-test PRIVATE ${LIB_EL_TEST_DIR})
target_include_directories(lib-el-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
target_link_libraries(lib-el-test PRIVATE lib-el lib-kdl-test_utils lib-el-test_utils Catch2::Catch2WithMain fmt::fmt-header-only)
target_link_libraries(lib-el-test PRIVATE compiler-config precompile-std-headers)
