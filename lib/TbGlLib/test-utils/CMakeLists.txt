add_library(TbGlTestUtilsLib INTERFACE)

target_sources(TbGlTestUtilsLib
  INTERFACE FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbGlTestUtilsLib INTERFACE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbGlTestUtilsLib INTERFACE TbGlLib)
