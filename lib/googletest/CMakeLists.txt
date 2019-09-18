set(LIB_GTEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_GTEST_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_GTEST_SOURCE "${LIB_GTEST_SOURCE_DIR}/gtest-all.cc")

add_library(gtest ${LIB_GTEST_SOURCE})
target_compile_options(gtest PRIVATE -w) # supress all warnings

# Work around for gtest bug, see: https://stackoverflow.com/questions/21116622/undefined-reference-to-pthread-key-create-linker-error
target_link_libraries(gtest PUBLIC Threads::Threads)

target_include_directories(gtest
        SYSTEM PUBLIC ${LIB_GTEST_INCLUDE_DIR})
