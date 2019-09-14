set(LIB_GTEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_GTEST_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_GTEST_SOURCE "${LIB_GTEST_SOURCE_DIR}/gtest-all.cc")

add_library(gtest ${LIB_GTEST_SOURCE})
target_compile_options(gtest PRIVATE -w) # supress all warnings
target_include_directories(gtest
        PRIVATE ${LIB_GTEST_INCLUDE_DIR}
        SYSTEM PUBLIC
        $<BUILD_INTERFACE:${LIB_GTEST_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:gtest/include/gtest>)
