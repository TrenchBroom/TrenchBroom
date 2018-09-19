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
	ADD_LIBRARY(common STATIC ${COMMON_SOURCE} ${COMMON_HEADER})
	SET_XCODE_ATTRIBUTES(common)
#ENDIF()

INCLUDE_DIRECTORIES(${COMMON_SOURCE_DIR})

SET_TARGET_PROPERTIES(common PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

# cotire
set_target_properties(common PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "Prefix.h")
cotire(common)
