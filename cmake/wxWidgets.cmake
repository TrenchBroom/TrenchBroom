IF(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "No build type selected, default to Release")
	SET(CMAKE_BUILD_TYPE "Release")
ELSE()
	MESSAGE(STATUS "Build type ${CMAKE_BUILD_TYPE}")
ENDIF()

IF(WIN32)
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(wxWidgets_USE_DEBUG ON)
        SET(wxWidgets_CONFIGURATION "mswud")
    ELSE()
        SET(wxWidgets_USE_DEBUG OFF)
        SET(wxWidgets_CONFIGURATION "mswu")
    ENDIF()
ELSE()
    SET(wxWidgets_USE_UNICODE ON)
    SET(wxWidgets_USE_UNIVERSAL ON)
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(wxWidgets_USE_DEBUG ON)
        SET(wxWidgets_USE_STATIC OFF)
    ELSE()
        SET(wxWidgets_USE_DEBUG OFF)
        SET(wxWidgets_USE_STATIC ON)
    ENDIF()
ENDIF()

SET(wxWidgets_FIND_COMPONENTS gl core base adv)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
FIND_PACKAGE(wxWidgetsTB REQUIRED)
INCLUDE("${wxWidgets_USE_FILE}")

# Check version
message(STATUS "wxWidgets ${wxWidgets_VERSION_MAJOR}.${wxWidgets_VERSION_MINOR}.${wxWidgets_VERSION_PATCH} found.")

SET(TB_WX_MAJOR 3)
SET(TB_WX_MINOR 1)
SET(TB_WX_PATCH 1)

IF ((${wxWidgets_VERSION_MAJOR} LESS ${TB_WX_MAJOR}) OR
(${wxWidgets_VERSION_MAJOR} EQUAL ${TB_WX_MAJOR} AND ${wxWidgets_VERSION_MINOR} LESS ${TB_WX_MINOR}) OR
(${wxWidgets_VERSION_MAJOR} EQUAL ${TB_WX_MAJOR} AND ${wxWidgets_VERSION_MINOR} EQUAL ${TB_WX_MINOR} AND ${wxWidgets_VERSION_PATCH} LESS ${TB_WX_PATCH}))
    message(FATAL_ERROR "wxWidgets ${TB_WX_MAJOR}.${TB_WX_MINOR}.${TB_WX_PATCH} required.")
ENDIF()

# Remove QuickTime framework on OS X; it's not needed and produces a linker warning
STRING(REPLACE "-framework QuickTime;" "" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")

