# Mac OS X specific configuration
IF(APPLE)
	SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
	SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk/")
	SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(TrenchBroom)

IF(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "No build type selected, default to Release")
	SET(CMAKE_BUILD_TYPE "Release")
ELSE()
	MESSAGE(STATUS "Build type ${CMAKE_BUILD_TYPE}")
ENDIF()

# wxWidgets configuration
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	SET(wxWidgets_USE_DEBUG)
ENDIF()

IF(NOT CMAKE_BUILD_TYPE MATCHES "Debug" AND NOT WINDOWS)
	SET(wxWidgets_USE_STATIC)
ENDIF()

IF(wxWidgets_USE_STATIC)
	MESSAGE(STATUS "Using static wxWidgets library")
ENDIF()

SET(wxWidgets_USE_UNICODE)
SET(wxWidgets_USE_UNIVERSAL)
SET(wxWidgets_USE_LIBS)
FIND_PACKAGE(wxWidgets REQUIRED gl core base)
INCLUDE("${wxWidgets_USE_FILE}")

# Remove QuickTime framework on OS X; it's not needed and produces a linker warning
STRING(REPLACE "-framework QuickTime;" "" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")

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

# Configure freetype library
IF(APPLE)
	SET(FREETYPE_LIBRARIES "${LIB_BIN_DIR}/osx/libfreetype.a" "${LIB_BIN_DIR}/osx/libbz2.a")
	FIND_PATH(FREETYPE_INCLUDE_DIRS ft2build.h "${LIB_INCLUDE_DIR}" DOC "Freetype includes")
ELSE()
	INCLUDE(FindFreetype)
	IF((NOT FREETYPE_FOUND) AND WIN32)
  		SET(FREETYPE_LIBRARIES "${LIB_BIN_DIR}/win32/freetype.lib" "${LIB_BIN_DIR}/win32/freetype6.dll")
  		FIND_PATH(FREETYPE_INCLUDE_DIRS ft2build.h "${LIB_INCLUDE_DIR}" DOC "Freetype includes")
	ENDIF()
ENDIF()
INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})

# Configure gtest library
SET(LIB_GTEST_SOURCE_DIR "${LIB_SOURCE_DIR}/gtest")
SET(LIB_GTEST_SOURCE "${LIB_GTEST_SOURCE_DIR}/gtest-all.cc")
SET_SOURCE_FILES_PROPERTIES(${LIB_GTEST_SOURCE} PROPERTIES COMPILE_FLAGS "-w")

# Configure gtest library
SET(LIB_GMOCK_SOURCE_DIR "${LIB_SOURCE_DIR}/gmock")
SET(LIB_GMOCK_SOURCE "${LIB_GMOCK_SOURCE_DIR}/gmock-all.cc")
SET_SOURCE_FILES_PROPERTIES(${LIB_GMOCK_SOURCE} PROPERTIES COMPILE_FLAGS "-w")

# Configure glew library
SET(LIB_GLEW_SOURCE_DIR "${LIB_SOURCE_DIR}/glew")
SET(LIB_GLEW_SOURCE "${LIB_GLEW_SOURCE_DIR}/glew.c")
SET_SOURCE_FILES_PROPERTIES(${LIB_GLEW_SOURCE} PROPERTIES COMPILE_FLAGS "-w")

ADD_LIBRARY(gtest ${LIB_GTEST_SOURCE} ${LIB_INCLUDE_DIR})
ADD_LIBRARY(gmock ${LIB_GMOCK_SOURCE} ${LIB_INCLUDE_DIR})
ADD_LIBRARY(glew ${LIB_GLEW_SOURCE} ${LIB_INCLUDE_DIR})

# Include directories
INCLUDE_DIRECTORIES("${LIB_INCLUDE_DIR}" "${SOURCE_DIR}" "${TEST_SOURCE_DIR}")

# Compiler configuration
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "/W3 /EHsc /MP /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE")
	SET(CMAKE_C_FLAGS "/W3 /MP")
ELSE()
	SET(CMAKE_CXX_FLAGS_RELEASE "-Os")
	SET(CMAKE_CXX_FLAGS "-Wall -pedantic -std=c++0x")

	# Xcode requires these flags to allow debugging
	SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fno-inline")

	# Hide some bogus warnings and enable color diagnostics if ccache is used with clang
	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		SET(CMAKE_CXX_FLAGS "-Qunused-arguments -fcolor-diagnostics")
	ENDIF()
ENDIF()

# TrenchBroom executable
# OS X app bundle configuration
IF(APPLE)
	# Configure icons
	SET(MACOSX_ICON_FILES "${CMAKE_SOURCE_DIR}/resources/graphics/AppIcon.icns" "${CMAKE_SOURCE_DIR}/resources/graphics/DocIcon.icns")
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	SET(SOURCE ${SOURCE} ${MACOSX_ICON_FILES})

	# Configure game resources
	# Collect all game resources
	FILE(GLOB_RECURSE MACOSX_GAME_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/quake/*.*"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_GAME_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/quake)
	SET(SOURCE ${SOURCE} ${MACOSX_GAME_FILES})

	# Configure shaders
	# Collect all shaders
	FILE(GLOB_RECURSE MACOSX_SHADER_FILES
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.fragsh"
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.vertsh"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
	SET(SOURCE ${SOURCE} ${MACOSX_SHADER_FILES})
ENDIF()

ADD_EXECUTABLE(TrenchBroom WIN32 MACOSX_BUNDLE ${SOURCE})
TARGET_LINK_LIBRARIES(TrenchBroom ${wxWidgets_LIBRARIES} glew ${FREETYPE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")
IF(APPLE)
	# Configure plist file
	SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/mac/TrenchBroom-Info.plist")
ENDIF()

IF(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Copy game files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/games/" $<TARGET_FILE_DIR:TrenchBroom>/Resources/
	)

	# Copy shader files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/shader" $<TARGET_FILE_DIR:TrenchBroom>/Resources/shader
	)
ENDIF()

# TrenchBroom-Test executable
ADD_EXECUTABLE(TrenchBroom-Test ${SOURCE} ${TEST_SOURCE})
TARGET_LINK_LIBRARIES(TrenchBroom-Test ${wxWidgets_LIBRARIES} gtest gmock ${FREETYPE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES COMPILE_DEFINITIONS "TESTING")


ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test/data" $<TARGET_FILE_DIR:TrenchBroom-Test>/data
)

IF(WIN32)
	# Copy DLLs to app directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" $<TARGET_FILE_DIR:TrenchBroom>
	)
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" $<TARGET_FILE_DIR:TrenchBroom-Test>
	)
ENDIF()
