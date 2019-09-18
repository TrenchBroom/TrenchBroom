set(LIB_GMOCK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_GMOCK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_GMOCK_SOURCE "${LIB_GMOCK_SOURCE_DIR}/gmock-all.cc")

add_library(gmock ${LIB_GMOCK_SOURCE})
target_link_libraries(gmock gtest)
target_compile_options(gmock PRIVATE -w) # supress all warnings
target_include_directories(gmock
        SYSTEM PUBLIC ${LIB_GMOCK_INCLUDE_DIR})
