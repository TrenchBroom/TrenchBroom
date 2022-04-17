#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# freeimage_FOUND
# freeimage_INCLUDE_PATH
# freeimage_LIBRARY
#

find_path(freeimage_INCLUDE_DIR
  NAMES FreeImage.h
  PATHS
  /usr/include
  /usr/local/include
  /sw/include
  /opt/local/include)
find_library(freeimage_LIBRARY
  NAMES FreeImage freeimage
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freeimage
  FOUND_VAR freeimage_FOUND
  REQUIRED_VARS
    freeimage_LIBRARY
    freeimage_INCLUDE_DIR
)

if(freeimage_FOUND)
  set(freeimage_LIBRARIES ${freeimage_LIBRARY})
  set(freeimage_INCLUDE_DIRS ${freeimage_INCLUDE_DIR})
endif()

if(freeimage_FOUND AND NOT TARGET freeimage::FreeImage)
  add_library(freeimage::FreeImage UNKNOWN IMPORTED)
  set_target_properties(freeimage::FreeImage PROPERTIES
    IMPORTED_LOCATION "${freeimage_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${freeimage_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  freeimage_INCLUDE_DIR
  freeimage_LIBRARY
)
