add_executable(TbElLibTest)

target_sources(TbElLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EL.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ELParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Expression.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Interpolate.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbElLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbElLibTest PRIVATE Catch2::Catch2WithMain TbElLib TbBaseTestUtilsLib TbElTestUtilsLib)
