set(LIB_BASE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_BASE_TEST
        "${LIB_BASE_TEST_DIR}/tst_Color.cpp"
        "${LIB_BASE_TEST_DIR}/tst_ColorComponentType.cpp"
        "${LIB_BASE_TEST_DIR}/tst_ColorT.cpp"
        "${LIB_BASE_TEST_DIR}/tst_ColorVariantT.cpp"
        "${LIB_BASE_TEST_DIR}/tst_Ensure.cpp"
        "${LIB_BASE_TEST_DIR}/tst_Notifier.cpp"
        "${LIB_BASE_TEST_DIR}/tst_Tokenizer.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

add_executable(lib-base-test ${LIB_BASE_TEST})
target_include_directories(lib-base-test PRIVATE ${LIB_BASE_TEST_DIR})
target_link_libraries(lib-base-test PRIVATE lib-base Catch2::Catch2WithMain fmt::fmt-header-only)
target_link_libraries(lib-base-test PRIVATE compiler-config precompile-std-headers)
