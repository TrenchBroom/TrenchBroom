SET(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)

FILE(GLOB_RECURSE COMMON_SOURCE
    "${COMMON_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE COMMON_HEADER
    "${COMMON_SOURCE_DIR}/*.h"
)

# Unfortunately, Xcode still compiles OBJECT libraries as static libraries, so there's no real gain in build time.
# But we can still use this on other platforms and in Release builds
#IF(NOT CMAKE_GENERATOR STREQUAL "Xcode")
	ADD_LIBRARY(common OBJECT ${COMMON_SOURCE} ${COMMON_HEADER})
	SET_XCODE_ATTRIBUTES(common)
#ENDIF()

INCLUDE_DIRECTORIES(${COMMON_SOURCE_DIR})

SET_TARGET_PROPERTIES(common PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

# Create the cmake script for generating the version information
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake" @ONLY)
ADD_TARGET_PROPERTY(common INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(GenerateVersion
		${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake")
ADD_DEPENDENCIES(common GenerateVersion)

# cotire
set_target_properties(common PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "Prefix.h")
cotire(common)
