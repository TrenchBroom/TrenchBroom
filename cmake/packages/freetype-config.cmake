#
# Try to find the FreeType library and include path.
# Once done this will define
#
# freetype_FOUND
# freetype_INCLUDE_PATH
# freetype_LIBRARY
#

find_path(freetype_INCLUDE_DIR
  NAMES ft2build.h
  PATHS
  /usr/include
  /usr/include/freetype2
  /usr/local/include
  /sw/include
  /opt/local/include)
find_library(freetype_LIBRARY
  NAMES FreeType freetype
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freetype
  FOUND_VAR freetype_FOUND
  REQUIRED_VARS
    freetype_LIBRARY
    freetype_INCLUDE_DIR
)

if(freetype_FOUND)
  set(freetype_LIBRARIES ${freetype_LIBRARY})
  set(freetype_INCLUDE_DIRS ${freetype_INCLUDE_DIR})
endif()

if(freetype_FOUND AND NOT TARGET freetype)
  add_library(freetype UNKNOWN IMPORTED)
  set_target_properties(freetype PROPERTIES
    IMPORTED_LOCATION "${freetype_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${freetype_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  freetype_INCLUDE_DIR
  freetype_LIBRARY
)
