add_executable(TbElLibTest)
EMBED_UTF8_MANIFEST(TbElLibTest)

target_sources(TbElLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Expression.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Interpolate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Value.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_VariableStore.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbElLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbElLib
    TbBaseTestUtilsLib
    TbElTestUtilsLib
)

include(Catch)
catch_discover_tests(TbElLibTest)
