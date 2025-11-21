add_library(TbElTestUtilsLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ELTestUtils.cpp
)

target_include_directories(TbElTestUtilsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbElTestUtilsLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbElTestUtilsLib PRIVATE TbElLib)
