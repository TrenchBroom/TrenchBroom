add_library(optlite INTERFACE)
target_compile_features(optlite INTERFACE cxx_std_17)
target_include_directories(optlite
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:optional-lite/include/optional-lite>)
