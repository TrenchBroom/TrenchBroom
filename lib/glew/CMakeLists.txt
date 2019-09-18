set(LIB_GLEW_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_GLEW_SOURCE "${LIB_GLEW_SOURCE_DIR}/glew.c")

add_library(glew STATIC ${LIB_GLEW_SOURCE})
target_compile_options(glew PRIVATE -w) # supress all warnings
target_compile_definitions(glew PUBLIC GLEW_STATIC)
target_include_directories(glew
        SYSTEM PUBLIC ${LIB_GLEW_INCLUDE_DIR})
