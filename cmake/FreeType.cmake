IF(APPLE)
	# Must set the path manually because otherwise it will find a 64bit only freetype lib that comes with OS X

	FIND_PATH(FREETYPE_INCLUDE_PATH ft2build.h "${LIB_INCLUDE_DIR}" DOC "Freetype includes")
	SET(FREETYPE_LIBRARY "${LIB_BIN_DIR}/osx/libfreetype.a")
	
	SET(FREETYPE_LIBRARIES 
		${FREETYPE_LIBRARY} 
  		"${LIB_BIN_DIR}/osx/libbz2.a" 
  		"${LIB_BIN_DIR}/osx/libz.a")
ELSEIF(MSVC)
	FIND_PATH(FREETYPE_INCLUDE_PATH ft2build.h "${LIB_INCLUDE_DIR}" DOC "Freetype includes")
	SET(FREETYPE_LIBRARY "${LIB_BIN_DIR}/win32/freetype.lib")
	
	SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
ELSEIF(MINGW)
	FIND_PATH(FREETYPE_INCLUDE_PATH ft2build.h "${LIB_INCLUDE_DIR}" DOC "Freetype includes")
	SET(FREETYPE_LIBRARY "${LIB_BIN_DIR}/win32/libfreetype.a")
	
	SET(FREETYPE_LIBRARIES 
		${FREETYPE_LIBRARY} 
  		"${LIB_BIN_DIR}/win32/libz.a")
ELSE()
	FIND_PATH( FREETYPE_INCLUDE_PATH ft2build.h
		/usr/include
		/usr/include/freetype2
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where ft2build.h resides")
	FIND_LIBRARY( FREETYPE_LIBRARY
		NAMES FreeType freetype
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The FreeType library")
	SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
ENDIF()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FREETYPE  DEFAULT_MSG  FREETYPE_LIBRARY  FREETYPE_INCLUDE_PATH)
MARK_AS_ADVANCED(
	FREETYPE_FOUND 
	FREETYPE_LIBRARY
	FREETYPE_LIBRARIES
	FREETYPE_INCLUDE_PATH)
