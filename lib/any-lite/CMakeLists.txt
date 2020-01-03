add_library(anylite INTERFACE)
if(APPLE)
    # macOS includes a broken <any> header unless the deployment taret is >= 10.14.
    # This prevents any-lite from using the system's broken <any> header.
    target_compile_definitions(anylite INTERFACE -Dany_CONFIG_SELECT_ANY=any_ANY_NONSTD)
endif()
target_compile_features(anylite INTERFACE cxx_std_17)
target_include_directories(anylite
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:any-lite/include/nonstd>)
