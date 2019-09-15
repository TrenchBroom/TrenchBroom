# Using TARGET_LINK_LIBRARY to link an object library to qt requires 3.12
cmake_minimum_required(VERSION 3.12)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Using CCache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Fix warning with Ninja and cotire (see https://github.com/sakra/cotire/issues/81)
if(POLICY CMP0058)
    cmake_policy(SET CMP0058 NEW)
endif()

# Using C and CXX because GLEW is C
project(TrenchBroom C CXX)

# Compiler detection
set(COMPILER_IS_CLANG FALSE)
set(COMPILER_IS_GNU FALSE)
set(COMPILER_IS_MSVC FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(COMPILER_IS_CLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_IS_GNU TRUE)
elseif(MSVC EQUAL 1)
    set(COMPILER_IS_MSVC TRUE)
else()
    message(FATAL_ERROR "Unsupported compiler detected.")
endif()

include(cmake/Utils.cmake)

set(APP_DIR "${CMAKE_SOURCE_DIR}/app")
set(APP_RESOURCE_DIR "${APP_DIR}/resources")

add_subdirectory(lib/googletest)
add_subdirectory(lib/googlemock)

add_subdirectory(lib/optional-lite)
add_subdirectory(lib/glew)
add_subdirectory(lib/freeimage)
add_subdirectory(lib/freetype)
add_subdirectory(lib/tinyxml2)
add_subdirectory(lib/miniz)
add_subdirectory(lib/vecmath)

add_subdirectory(common)
