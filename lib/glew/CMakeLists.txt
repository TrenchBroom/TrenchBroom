set(LIB_GLEW_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_GLEW_SOURCE "${LIB_GLEW_SOURCE_DIR}/glew.c")

add_library(glew ${LIB_GLEW_SOURCE})
target_compile_options(glew PRIVATE -w) # supress all warnings
target_compile_definitions(glew PRIVATE GLEW_STATIC)
target_include_directories(glew
        SYSTEM PUBLIC
        $<BUILD_INTERFACE:${LIB_GLEW_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:glew/include/GL>)
