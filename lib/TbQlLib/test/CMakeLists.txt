add_executable(TbQlLibTest)
EMBED_UTF8_MANIFEST(TbQlLibTest)

target_sources(TbQlLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushNodeBinding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityNodeBinding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GroupNodeBinding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LayerNodeBinding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeQueryValues.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PatchNodeBinding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_WorldNodeBinding.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbQlLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbBaseTestUtilsLib
    TbElTestUtilsLib
    TbMdlLib
    TbMdlTestUtilsLib
    TbQlLib
    TbVersionLib
)

include(Catch)
catch_discover_tests(TbQlLibTest)
