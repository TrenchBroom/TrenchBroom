set(LIB_MINIZ_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_MINIZ_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_MINIZ_SOURCE "${LIB_MINIZ_SOURCE_DIR}/miniz.c")

add_library(miniz STATIC ${LIB_MINIZ_SOURCE})
target_compile_options(miniz PRIVATE -w) # supress all warnings
target_include_directories(miniz
        SYSTEM INTERFACE ${LIB_MINIZ_INCLUDE_DIR}
        PRIVATE ${LIB_MINIZ_INCLUDE_DIR}/miniz)
