#
# Try to find the tinyxml2 library and include path.
# Once done this will define
#
# tinyxml2_FOUND
# tinyxml2_INCLUDE_PATH
# tinyxml2_LIBRARY
#

find_path(tinyxml2_INCLUDE_DIR
  NAMES tinyxml2.h
  PATHS
  /usr/include
  /usr/local/include
  /sw/include
  /opt/local/include)
find_library(tinyxml2_LIBRARY
  NAMES tinyxml2
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tinyxml2
  FOUND_VAR tinyxml2_FOUND
  REQUIRED_VARS
    tinyxml2_LIBRARY
    tinyxml2_INCLUDE_DIR
)

if(tinyxml2_FOUND)
  set(tinyxml2_LIBRARIES ${tinyxml2_LIBRARY})
  set(tinyxml2_INCLUDE_DIRS ${tinyxml2_INCLUDE_DIR})
endif()

if(tinyxml2_FOUND AND NOT TARGET tinyxml2::tinyxml2)
  add_library(tinyxml2::tinyxml2 UNKNOWN IMPORTED)
  set_target_properties(tinyxml2::tinyxml2 PROPERTIES
    IMPORTED_LOCATION "${tinyxml2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${tinyxml2_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  tinyxml2_INCLUDE_DIR
  tinyxml2_LIBRARY
)
