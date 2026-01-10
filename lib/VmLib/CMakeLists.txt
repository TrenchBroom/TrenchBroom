cmake_minimum_required(VERSION 3.12)

project(VmLib VERSION 0.1.0 LANGUAGES CXX)

add_library(VmLib INTERFACE)

target_sources(VmLib
  INTERFACE FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(VmLib INTERFACE CompilerConfig)

# Use the fast-float library for from_chars on AppleClang, where std::from_chars is not
# available for floating point types.
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  target_link_libraries(VmLib INTERFACE FastFloat::fast_float)
endif()

add_subdirectory(test)
