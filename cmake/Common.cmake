SET(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)

FILE(GLOB_RECURSE COMMON_SOURCE
    "${COMMON_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE COMMON_HEADER
    "${COMMON_SOURCE_DIR}/*.h"
)

# Unfortunately, Xcode still compiles OBJECT libraries as static libraries
SET(TB_COMMON_LIBRARY_TYPE OBJECT)
IF(CMAKE_GENERATOR STREQUAL "Xcode" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(TB_COMMON_LIBRARY_TYPE SHARED)
ENDIF()
MESSAGE(STATUS "Building common as ${TB_COMMON_LIBRARY_TYPE} library")

ADD_LIBRARY(common ${TB_COMMON_LIBRARY_TYPE} ${COMMON_SOURCE} ${COMMON_HEADER})
SET_XCODE_ATTRIBUTES(common)

# Configure dependencies if building a shared library.
get_target_property(common_TYPE common TYPE)
IF(common_TYPE STREQUAL "SHARED_LIBRARY")
	IF(COMPILER_IS_GNU AND TB_ENABLE_ASAN)
		TARGET_LINK_LIBRARIES(common asan)
	ENDIF()

	TARGET_LINK_LIBRARIES(common glew ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES} vecmath)
ENDIF()

INCLUDE_DIRECTORIES(${COMMON_SOURCE_DIR})

SET_TARGET_PROPERTIES(common PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

# Create the cmake script for generating the version information

# Need to find git first because GenerateVersion.cmake.in accesses the GIT_EXECUTABLE variable it populates.
FIND_PACKAGE(Git)
IF (NOT GIT_FOUND)
    MESSAGE(WARNING "Could not find git")
ENDIF()

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake" @ONLY)
ADD_TARGET_PROPERTY(common INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(GenerateVersion
		${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake")
ADD_DEPENDENCIES(common GenerateVersion)
