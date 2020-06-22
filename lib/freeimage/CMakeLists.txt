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
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}/win32" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/osx/libfreeimage.a")

    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
elseif(MSVC)
    if(CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
        set(FREEIMAGE_ARCH win32)
    elseif(CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
        set(FREEIMAGE_ARCH win64)
    else()
        message(ERROR "Unsupported CMAKE_VS_PLATFORM_NAME ${CMAKE_VS_PLATFORM_NAME}")
    endif()
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}/${FREEIMAGE_ARCH}" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/${FREEIMAGE_ARCH}/FreeImage.dll")
    set(FREEIMAGE_IMP_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/${FREEIMAGE_ARCH}/FreeImage.lib")

    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY} ${FREEIMAGE_IMP_LIBRARY})
elseif(MINGW)
    find_path(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_FREEIMAGE_INCLUDE_DIR}/win32" DOC "Freeimage includes")
    set(FREEIMAGE_LIBRARY "${LIB_FREEIMAGE_BINARY_DIR}/win32/libfreeimage.a")

    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
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

    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage  DEFAULT_MSG  FREEIMAGE_LIBRARY  FREEIMAGE_INCLUDE_PATH)

mark_as_advanced(
        FREEIMAGE_FOUND
        FREEIMAGE_LIBRARY
        FREEIMAGE_LIBRARIES
        FREEIMAGE_INCLUDE_PATH)

if(FREEIMAGE_FOUND)
    if(MSVC)
        add_library(freeimage SHARED IMPORTED GLOBAL)
        set_target_properties(freeimage PROPERTIES IMPORTED_IMPLIB ${FREEIMAGE_IMP_LIBRARY})
    else()
        add_library(freeimage STATIC IMPORTED GLOBAL)
    endif()
    set_target_properties(freeimage PROPERTIES IMPORTED_LOCATION ${FREEIMAGE_LIBRARY})
    target_include_directories(freeimage SYSTEM INTERFACE ${FREEIMAGE_INCLUDE_PATH})
endif()
