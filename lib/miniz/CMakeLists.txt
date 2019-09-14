set(LIB_MINIZ_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_MINIZ_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_MINIZ_SOURCE "${LIB_MINIZ_SOURCE_DIR}/miniz.c")

add_library(miniz ${LIB_MINIZ_SOURCE})
target_compile_options(miniz PRIVATE -w) # supress all warnings
target_include_directories(miniz
        PRIVATE ${LIB_MINIZ_INCLUDE_DIR}/miniz
        SYSTEM PUBLIC
        $<BUILD_INTERFACE:${LIB_MINIZ_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:miniz/include/miniz>)
