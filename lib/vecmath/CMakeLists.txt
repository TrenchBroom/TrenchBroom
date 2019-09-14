set(VECMATH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(vecmath INTERFACE)
target_compile_features(vecmath INTERFACE cxx_std_17)
target_include_directories(vecmath INTERFACE
        $<BUILD_INTERFACE:${VECMATH_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:vecmath/include/vecmath>)

add_subdirectory(test)
