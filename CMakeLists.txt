# Mac OS X specific configuration
IF(APPLE)
	SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
	SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk/")
	SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
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
INCLUDE_DIRECTORIES("${LIB_INCLUDE_DIR}" "${SOURCE_DIR}" "${TEST_SOURCE_DIR}")

# Compiler configuration
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /MP /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /MP")
ELSE()
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wno-format -std=c++0x")

	# Xcode requires these flags to allow debugging
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -fno-inline")

	# Hide some bogus warnings and enable color diagnostics if ccache is used with clang
	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -fcolor-diagnostics")
	ENDIF()
ENDIF()

# Set debug mode directives
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	ADD_DEFINITIONS(-DWXDEBUG -DDEBUG)
ENDIF()

# TrenchBroom executable
# OS X app bundle configuration
IF(APPLE)
	# Configure icons
	SET(MACOSX_ICON_FILES "${CMAKE_SOURCE_DIR}/resources/graphics/icons/AppIcon.icns" "${CMAKE_SOURCE_DIR}/resources/graphics/icons/DocIcon.icns")
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	SET(SOURCE ${SOURCE} ${MACOSX_ICON_FILES})

	# Configure button bitmaps etc.
	FILE(GLOB_RECURSE MACOSX_IMAGE_FILES
	    "${CMAKE_SOURCE_DIR}/resources/graphics/images/*.png"
	)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_IMAGE_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/images)
	SET(SOURCE ${SOURCE} ${MACOSX_IMAGE_FILES})

	# Configure game resources
	# Collect all game resources
	FILE(GLOB_RECURSE MACOSX_QUAKE_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Quake/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_QUAKE2_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Quake2/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_HEXEN2_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Hexen2/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_GAME_CONFIG_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/*.cfg"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake2)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_HEXEN2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Hexen2)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_GAME_CONFIG_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games)
	SET(SOURCE ${SOURCE} ${MACOSX_QUAKE_FILES} ${MACOSX_QUAKE2_FILES} ${MACOSX_HEXEN2_FILES} ${MACOSX_GAME_CONFIG_FILES})

	# Configure shaders
	# Collect all shaders
	FILE(GLOB_RECURSE MACOSX_SHADER_FILES
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.fragsh"
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.vertsh"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
	SET(SOURCE ${SOURCE} ${MACOSX_SHADER_FILES})
ELSEIF(MSVC)
  	SET(SOURCE ${SOURCE} "${SOURCE_DIR}/TrenchBroom.rc")
ELSEIF(MINGW)
	SET(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -i <SOURCE> -o <OBJECT>")
ENDIF()

ADD_EXECUTABLE(TrenchBroom WIN32 MACOSX_BUNDLE ${SOURCE})
TARGET_LINK_LIBRARIES(TrenchBroom ${wxWidgets_LIBRARIES} glew ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

IF(APPLE)
	# Configure plist file
	SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/mac/TrenchBroom-Info.plist")
	# Set Debug information format
	SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf-with-dsym")
ENDIF()

IF(WIN32)
	# Copy Windows icons to target dir
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/graphics/icons/TrenchBroom.ico" $CMAKE_CURRENT_BINARY_DIR
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/graphics/icons/TrenchBroomDoc.ico" $CMAKE_CURRENT_BINARY_DIR
	)
ENDIF()

IF(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Copy button images to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/graphics/images" $<TARGET_FILE_DIR:TrenchBroom>/Resources/images
	)

	# Copy game files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/games/" $<TARGET_FILE_DIR:TrenchBroom>/Resources/games
	)

	# Copy shader files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/shader" $<TARGET_FILE_DIR:TrenchBroom>/Resources/shader
	)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FIND_PACKAGE(OpenGL)
	TARGET_LINK_LIBRARIES(TrenchBroom ${OPENGL_LIBRARIES})
ENDIF()

# TrenchBroom-Test executable
ADD_EXECUTABLE(TrenchBroom-Test ${SOURCE} ${TEST_SOURCE})
TARGET_LINK_LIBRARIES(TrenchBroom-Test ${wxWidgets_LIBRARIES} gtest gmock ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES COMPILE_DEFINITIONS TESTING=1)

# Disable some GCC warnings when compiling the tests
IF(CMAKE_COMPILER_IS_GNUCXX)
	SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES COMPILE_FLAGS -Wno-unused-local-typedefs)
ENDIF()

IF(APPLE)
	# Set Debug information format
	SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf-with-dsym")
ENDIF()

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
