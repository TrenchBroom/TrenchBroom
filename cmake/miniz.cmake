SET(LIB_MINIZ_SOURCE_DIR "${LIB_SOURCE_DIR}/miniz")
SET(LIB_MINIZ_INCLUDE_DIR "${LIB_INCLUDE_DIR}/miniz")

SET(LIB_MINIZ_SOURCE "${LIB_MINIZ_SOURCE_DIR}/miniz.c"
                     "${LIB_MINIZ_INCLUDE_DIR}/miniz.h")

# Suppress all warnings in 3rd party code
SET_SOURCE_FILES_PROPERTIES(${LIB_MINIZ_SOURCE} PROPERTIES COMPILE_FLAGS "-w")

ADD_LIBRARY(miniz ${LIB_MINIZ_SOURCE} ${LIB_INCLUDE_DIR})

# miniz.c uses `#include  "miniz.h"`, but we've moved the header to lib/include/miniz
target_include_directories(miniz PRIVATE ${LIB_MINIZ_INCLUDE_DIR})
