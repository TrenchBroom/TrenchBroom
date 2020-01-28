add_library(variantlite INTERFACE)
if(APPLE)
    # macOS includes a broken <variant> header unless the deployment taret is >= 10.14.
    # This prevents variant-lite from using the system's broken <variant> header.
    target_compile_definitions(variantlite INTERFACE -Dvariant_CONFIG_SELECT_VARIANT=variant_VARIANT_NONSTD)
endif()
target_compile_features(variantlite INTERFACE cxx_std_17)
target_include_directories(variantlite
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:variant-lite/include/nonstd>)
