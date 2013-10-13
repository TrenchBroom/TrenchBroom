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
FIND_PACKAGE(wxWidgets REQUIRED gl core base adv)
INCLUDE("${wxWidgets_USE_FILE}")

# Remove QuickTime framework on OS X; it's not needed and produces a linker warning
STRING(REPLACE "-framework QuickTime;" "" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
