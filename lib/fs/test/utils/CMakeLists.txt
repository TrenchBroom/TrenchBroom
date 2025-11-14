set(LIB_FS_TEST_UTILS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_FS_TEST_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_FS_TEST_UTILS_HEADER
  ${LIB_FS_TEST_UTILS_INCLUDE_DIR}/fs/ObjectFile.h
  ${LIB_FS_TEST_UTILS_INCLUDE_DIR}/fs/TestEnvironment.h
  ${LIB_FS_TEST_UTILS_INCLUDE_DIR}/fs/TestFileSystem.h
)

set(LIB_FS_TEST_UTILS_SOURCE
  ${LIB_FS_TEST_UTILS_SOURCE_DIR}/TestEnvironment.cpp
  ${LIB_FS_TEST_UTILS_SOURCE_DIR}/TestFileSystem.cpp
)

add_library(lib-fs-test_utils STATIC ${LIB_FS_TEST_UTILS_SOURCE} ${LIB_FS_TEST_UTILS_HEADER})
target_include_directories(lib-fs-test_utils PUBLIC ${LIB_FS_TEST_UTILS_INCLUDE_DIR})
target_link_libraries(lib-fs-test_utils PRIVATE lib-fs)
target_link_libraries(lib-fs-test_utils PRIVATE compiler-config)
