#
# Try to find the FreeType library and include path.
# Once done this will define
#
# FREETYPE_FOUND
# FREETYPE_INCLUDE_PATH
# FREETYPE_LIBRARY
#

set(LIB_FREETYPE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(LIB_FREETYPE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(APPLE)
    # Must set the path manually because otherwise it will find a 64bit only freetype lib that comes with OS X
    set(FREETYPE_INCLUDE_PATH "${LIB_FREETYPE_INCLUDE_DIR}")
    set(FREETYPE_LIBRARY "${LIB_FREETYPE_BINARY_DIR}/osx/libfreetype.a")

    set(BZ2_LIBRARY "${LIB_FREETYPE_BINARY_DIR}/osx/libbz2.a")
    add_library(bz2 STATIC IMPORTED)
    set_target_properties(bz2 PROPERTIES IMPORTED_LOCATION ${BZ2_LIBRARY})

    set(Z_LIBRARY "${LIB_FREETYPE_BINARY_DIR}/osx/libz.a")
    add_library(z STATIC IMPORTED)
    set_target_properties(z PROPERTIES IMPORTED_LOCATION ${Z_LIBRARY})

    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY} ${BZ2_LIBRARY} ${Z_LIBRARY})
elseif(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL "4")
        set(FREETYPE_ARCH win32)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL "8")
        set(FREETYPE_ARCH win64)
    else()
        message(ERROR "Unsupported CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P}")
    endif()

    find_path(FREETYPE_INCLUDE_PATH ft2build.h "${CMAKE_CURRENT_SOURCE_DIR}/freetype-windows-binaries/include" DOC "Freetype includes")
    set(FREETYPE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/freetype-windows-binaries/${FREETYPE_ARCH}/freetype.dll")
    set(FREETYPE_IMP_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/freetype-windows-binaries/${FREETYPE_ARCH}/freetype.lib")

    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY} ${FREETYPE_IMP_LIBRARY})
elseif(MINGW)
    find_path(FREETYPE_INCLUDE_PATH ft2build.h "${LIB_FREETYPE_INCLUDE_DIR}" DOC "Freetype includes")
    set(FREETYPE_LIBRARY "${LIB_FREETYPE_BINARY_DIR}/win32/libfreetype.a")

    set(Z_LIBRARY "${LIB_FREETYPE_BINARY_DIR}/osx/libz.a")
    add_library(z STATIC IMPORTED)
    set_target_properties(z PROPERTIES IMPORTED_LOCATION ${Z_LIBRARY})

    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY} ${Z_LIBRARY})
else()
    find_path( FREETYPE_INCLUDE_PATH ft2build.h
            /usr/include
            /usr/include/freetype2
            /usr/local/include
            /sw/include
            /opt/local/include
            DOC "The directory where ft2build.h resides")
    find_library( FREETYPE_LIBRARY
            NAMES FreeType freetype
            PATHS
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            DOC "The FreeType library")
    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
ENDIF()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FREETYPE  DEFAULT_MSG  FREETYPE_LIBRARY  FREETYPE_INCLUDE_PATH)
mark_as_advanced(
        FREETYPE_FOUND
        FREETYPE_LIBRARY
        FREETYPE_LIBRARIES
        FREETYPE_INCLUDE_PATH)

if(FREETYPE_FOUND)
    if (MSVC)
        add_library(freetype SHARED IMPORTED GLOBAL)
        set_target_properties(freetype PROPERTIES IMPORTED_IMPLIB ${FREETYPE_IMP_LIBRARY})
    else()
        add_library(freetype STATIC IMPORTED GLOBAL)
    endif()
    set_target_properties(freetype PROPERTIES IMPORTED_LOCATION ${FREETYPE_LIBRARY})
    target_include_directories(freetype SYSTEM INTERFACE ${FREETYPE_INCLUDE_PATH})

    if(APPLE OR MINGW)
        target_link_libraries(freetype INTERFACE z)
    endif()
    if(APPLE)
        target_link_libraries(freetype INTERFACE bz2)
    endif()
endif()
