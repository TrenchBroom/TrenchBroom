#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
# 

IF(APPLE)
	# We want to link freeimage statically and not dynamically on OS X
	FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where FreeImage.h resides")
  	SET(FREEIMAGE_LIBRARY "${LIB_BIN_DIR}/osx/libfreeimage.a")
ELSEIF (WIN32)
	FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h "${LIB_INCLUDE_DIR}" DOC "Freeimage includes")
	SET(FREEIMAGE_LIBRARY "${LIB_BIN_DIR}/win32/FreeImage.lib")
ELSE()
	FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where FreeImage.h resides")
	FIND_LIBRARY( FREEIMAGE_LIBRARY
		NAMES FreeImage freeimage
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The FreeImage library")
ENDIF()

SET(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImage  DEFAULT_MSG  FREEIMAGE_LIBRARY  FREEIMAGE_INCLUDE_PATH)

MARK_AS_ADVANCED(
	FREEIMAGE_FOUND 
	FREEIMAGE_LIBRARY
	FREEIMAGE_LIBRARIES
	FREEIMAGE_INCLUDE_PATH)