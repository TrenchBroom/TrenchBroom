#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
#

set(LIB_FREEIMAGE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(LIB_FREEIMAGE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(APPLE)
    # We want to link freeimage statically and not dynamically on OS X
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/osx/libfreeimage.a")
elseif(MSVC)
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/win32/FreeImage.lib")
elseif(MINGW)
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/win32/libfreeimage.a")
else()
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h
            /usr/include
            /usr/local/include
            /sw/include
            /opt/local/include
            DOC "The directory where FreeImage.h resides")
    find_library(FREEIMAGE_LIBRARY
            NAMES FreeImage freeimage
            PATHS
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            DOC "The FreeImage library")
endif()

set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage  DEFAULT_MSG  FREEIMAGE_LIBRARY  FREEIMAGE_INCLUDE_PATH)

mark_as_advanced(
        FREEIMAGE_FOUND
        FREEIMAGE_LIBRARY
        FREEIMAGE_LIBRARIES
        FREEIMAGE_INCLUDE_PATH)

if(FREEIMAGE_FOUND)
    add_library(freeimage STATIC IMPORTED GLOBAL)
    set_target_properties(freeimage PROPERTIES IMPORTED_LOCATION ${FREEIMAGE_LIBRARY})
    target_include_directories(freeimage INTERFACE ${FREEIMAGE_INCLUDE_PATH})
endif()
