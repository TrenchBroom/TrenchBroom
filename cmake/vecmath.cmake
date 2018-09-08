SET(VECMATH_SOURCE_DIR ${CMAKE_SOURCE_DIR}/vecmath/src)

FILE(GLOB_RECURSE VECMATH_SOURCE
    "${VECMATH_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE VECMATH_HEADER
    "${VECMATH_SOURCE_DIR}/*.h"
)

# Unfortunately, Xcode still compiles OBJECT libraries as static libraries, so there's no real gain in build time.
# But we can still use this on other platforms and in Release builds
#IF(NOT CMAKE_GENERATOR STREQUAL "Xcode")
	ADD_LIBRARY(vecmath OBJECT ${VECMATH_SOURCE} ${VECMATH_HEADER})
	SET_XCODE_ATTRIBUTES(vecmath)
#ENDIF()

INCLUDE_DIRECTORIES(${VECMATH_SOURCE_DIR})
