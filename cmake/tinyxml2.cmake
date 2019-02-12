SET(LIB_TINYXML2_SOURCE_DIR "${LIB_SOURCE_DIR}/tinyxml2")
SET(LIB_TINYXML2_INCLUDE_DIR "${LIB_INCLUDE_DIR}/tinyxml2")

SET(LIB_TINYXML2_SOURCE "${LIB_TINYXML2_SOURCE_DIR}/tinyxml2.cpp"
                        "${LIB_TINYXML2_INCLUDE_DIR}/tinyxml2.h")

# Suppress all warnings in 3rd party code
SET_SOURCE_FILES_PROPERTIES(${LIB_TINYXML2_SOURCE} PROPERTIES COMPILE_FLAGS "-w")

ADD_LIBRARY(tinyxml2 ${LIB_TINYXML2_SOURCE} ${LIB_INCLUDE_DIR})

# tinyxml2.c uses `#include  "tinyxml2.h"`, but we've moved the header to lib/include/tinyxml2
target_include_directories(tinyxml2 PRIVATE ${LIB_TINYXML2_INCLUDE_DIR})
