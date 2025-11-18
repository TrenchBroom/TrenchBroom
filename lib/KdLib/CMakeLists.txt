cmake_minimum_required(VERSION 3.12)

project(KdLib VERSION 0.1.0 LANGUAGES CXX)

add_library(KdLib STATIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/src/cmd_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/dynamic_bitset.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/path_hash.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/path_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/regex_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/result_error.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/string_compare.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/string_format.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/string_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/task_manager.cpp"
)

target_include_directories(KdLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(KdLib PRIVATE CompilerConfig PrecompileStdHeaders)

# task_manager.h uses <thread>, etc., which requires this on Linux
find_package(Threads REQUIRED)
target_link_libraries(KdLib PUBLIC Threads::Threads)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  target_link_libraries(KdLib PRIVATE FastFloat::fast_float)
endif()

add_subdirectory(test)
