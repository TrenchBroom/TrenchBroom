cmake_minimum_required(VERSION 3.12)

project(cpp17 VERSION 0.1.0 LANGUAGES CXX)

# instruct cmake not to set default warning levels for MSVC projects (cmake 3.15 or higher)
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

set(CPP17_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(cpp17 INTERFACE)
target_compile_features(cpp17 INTERFACE cxx_std_17)
target_include_directories(cpp17 INTERFACE
        $<BUILD_INTERFACE:${CPP17_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:cpp17/include/>)

target_sources(cpp17 INTERFACE
    "${CPP17_INCLUDE_DIR}/any"
    "${CPP17_INCLUDE_DIR}/optional"
    "${CPP17_INCLUDE_DIR}/variant"
)

target_compile_features(cpp17 INTERFACE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(cpp17 INTERFACE -Wall -Wextra -Wconversion -pedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(cpp17 INTERFACE -Wall -Wextra -Wconversion -pedantic)
elseif(MSVC EQUAL 1)
    target_compile_options(cpp17 INTERFACE /W4 /EHsc /MP)

    # signed/unsigned mismatch: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4365
    target_compile_options(cpp17 INTERFACE /w44365)
else()
    message(FATAL_ERROR "Cannot set compile options for target")
endif()
