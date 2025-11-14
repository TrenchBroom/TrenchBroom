set(LIB_FS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_FS_TEST_SOURCE
        "${LIB_FS_TEST_DIR}/tst_DiskFileSystem.cpp"
        "${LIB_FS_TEST_DIR}/tst_DiskIO.cpp"
        "${LIB_FS_TEST_DIR}/tst_FileSystem.cpp"
        "${LIB_FS_TEST_DIR}/tst_ImageFileSystem.cpp"
        "${LIB_FS_TEST_DIR}/tst_VirtualFileSystem.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

add_subdirectory(utils)

add_executable(lib-fs-test ${LIB_FS_TEST_SOURCE})
target_include_directories(lib-fs-test PRIVATE ${LIB_FS_TEST_DIR})
target_include_directories(lib-fs-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
target_link_libraries(lib-fs-test PRIVATE lib-fs lib-fs-test_utils Catch2::Catch2WithMain fmt::fmt-header-only)
target_link_libraries(lib-fs-test PRIVATE compiler-config precompile-std-headers)
