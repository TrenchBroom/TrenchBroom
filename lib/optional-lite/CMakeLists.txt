add_library(optlite INTERFACE)
if(APPLE)
    # macOS includes a broken <optional> header unless the deployment taret is >= 10.14.
    # This prevents optional-lite from using the system's broken <optional> header.
    target_compile_definitions(optlite INTERFACE -Doptional_CONFIG_SELECT_OPTIONAL=optional_OPTIONAL_NONSTD)
endif()
target_compile_features(optlite INTERFACE cxx_std_17)
target_include_directories(optlite
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:optional-lite/include/nonstd>)
