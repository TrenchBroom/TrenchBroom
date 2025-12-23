add_library(TbGlTestUtilsLib INTERFACE)

target_include_directories(TbGlTestUtilsLib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:gl/include/gl>)

target_link_libraries(TbGlTestUtilsLib INTERFACE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbGlTestUtilsLib INTERFACE TbGlLib)
