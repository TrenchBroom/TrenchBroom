set(LIB_EL_TEST_UTILS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_EL_TEST_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_EL_TEST_UTILS_HEADER
  ${LIB_EL_TEST_UTILS_INCLUDE_DIR}/el/ELTestUtils.h
)

set(LIB_EL_TEST_UTILS_SOURCE
  ${LIB_EL_TEST_UTILS_SOURCE_DIR}/ELTestUtils.cpp
)

add_library(lib-el-test_utils STATIC ${LIB_EL_TEST_UTILS_SOURCE} ${LIB_EL_TEST_UTILS_HEADER})
target_include_directories(lib-el-test_utils PUBLIC ${LIB_EL_TEST_UTILS_INCLUDE_DIR})
target_link_libraries(lib-el-test_utils PRIVATE lib-el)
target_link_libraries(lib-el-test_utils PRIVATE compiler-config)
