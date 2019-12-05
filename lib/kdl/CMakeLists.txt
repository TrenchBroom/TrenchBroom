cmake_minimum_required(VERSION 3.12)

project(kdl VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(KDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(KDL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(KDL_SOURCE
        "${KDL_SOURCE_DIR}/string_compare.cpp")

add_library(kdl STATIC ${KDL_SOURCE})
target_compile_features(kdl PUBLIC cxx_std_17)
target_include_directories(kdl
        PUBLIC ${KDL_INCLUDE_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl PUBLIC -Wall -Wextra -pedantic -Wno-c++98-compat -Wno-padded)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl PUBLIC -Wall -Wextra -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(kdl PUBLIC /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(kdl PUBLIC /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# add_subdirectory(lib)
add_subdirectory(test)
