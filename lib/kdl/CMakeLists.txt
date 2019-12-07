cmake_minimum_required(VERSION 3.12)

project(kdl VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(KDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(kdl INTERFACE)
target_compile_features(kdl INTERFACE cxx_std_17)
target_include_directories(kdl INTERFACE
        $<BUILD_INTERFACE:${KDL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:kdl/include/kdl>)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(kdl INTERFACE -Wall -Wextra -Wconversion -pedantic -Wno-c++98-compat -Wno-padded)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kdl INTERFACE -Wall -Wextra -Wconversion -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(kdl INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(kdl INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# add_subdirectory(lib)
add_subdirectory(test)
