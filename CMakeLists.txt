# SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE(cmake/Utils.cmake)

# Mac OS X specific configuration, must be done before the PROJECT directive
IF(APPLE)
	SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
	SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk/")
	SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(TrenchBroom)

# Source directories
SET(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
SET(TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/test/src)

FILE(GLOB_RECURSE SOURCE
    "${SOURCE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE TEST_SOURCE
    "${TEST_SOURCE_DIR}/*.h"
    "${TEST_SOURCE_DIR}/*.cpp"
)

# Library configuration
SET(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
SET(LIB_BIN_DIR ${LIB_DIR}/bin)
SET(LIB_INCLUDE_DIR ${LIB_DIR}/include)
SET(LIB_SOURCE_DIR ${LIB_DIR}/src)

INCLUDE(cmake/wxWidgets.cmake)
INCLUDE(cmake/FreeType.cmake)
INCLUDE(cmake/FreeImage.cmake)

INCLUDE(cmake/GTest.cmake)
INCLUDE(cmake/GMock.cmake)
INCLUDE(cmake/Glew.cmake)

ADD_LIBRARY(gtest ${LIB_GTEST_SOURCE} ${LIB_INCLUDE_DIR})
ADD_LIBRARY(gmock ${LIB_GMOCK_SOURCE} ${LIB_INCLUDE_DIR})
ADD_LIBRARY(glew ${LIB_GLEW_SOURCE} ${LIB_INCLUDE_DIR})

# Include directories
INCLUDE_DIRECTORIES("${SOURCE_DIR}" "${TEST_SOURCE_DIR}")


# Compiler configuration
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /MP /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /MP")
    INCLUDE_DIRECTORIES(SYSTEM "${LIB_INCLUDE_DIR}")
ELSE()
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -pedantic -Wno-format -Wno-variadic-macros -Wno-c++11-long-long -Wno-c99-extensions -Wno-padded -Wno-unused-parameter -Wno-global-constructors -Wno-exit-time-destructors -Wno-weak-vtables -Wno-float-equal -Wno-used-but-marked-unused -Wno-format-nonliteral")

    IF(CMAKE_GENERATOR STREQUAL "Xcode")
        # Xcode requires these flags to allow debugging
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -fno-inline")

        # Xcode should treat the library header files as system headers
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${LIB_INCLUDE_DIR}")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${LIB_INCLUDE_DIR}")
    ELSE()
        INCLUDE_DIRECTORIES(SYSTEM "${LIB_INCLUDE_DIR}")
    ENDIF()
ENDIF()

# Set debug mode directives
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	ADD_DEFINITIONS(-DWXDEBUG -DDEBUG)
ENDIF()

INCLUDE(cmake/TrenchBroomApp.cmake)
INCLUDE(cmake/TrenchBroomTest.cmake)

