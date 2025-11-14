set(KDL_TEST_UTILS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(KDL_TEST_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(KDL_TEST_UTILS_HEADER
  ${KDL_TEST_UTILS_INCLUDE_DIR}/kdl/catch_string_makers.h
)

set(KDL_TEST_UTILS_SOURCE
)

add_library(lib-kdl-test_utils INTERFACE ${KDL_TEST_UTILS_SOURCE} ${KDL_TEST_UTILS_HEADER})
target_include_directories(lib-kdl-test_utils INTERFACE ${KDL_TEST_UTILS_INCLUDE_DIR})
target_link_libraries(lib-kdl-test_utils INTERFACE lib-kdl)
target_link_libraries(lib-kdl-test_utils INTERFACE compiler-config)
