set(LIB_TINYXML2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_TINYXML2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_TINYXML2_SOURCE "${LIB_TINYXML2_SOURCE_DIR}/tinyxml2.cpp")

add_library(tinyxml2 STATIC ${LIB_TINYXML2_SOURCE})
target_compile_options(tinyxml2 PRIVATE -w) # supress all warnings
target_include_directories(tinyxml2
        SYSTEM INTERFACE ${LIB_TINYXML2_INCLUDE_DIR}
        PRIVATE ${LIB_TINYXML2_INCLUDE_DIR}/tinyxml2)
