add_executable(TbQlLibTest)
EMBED_UTF8_MANIFEST(TbQlLibTest)

target_sources(TbQlLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushNodeLazyMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityNodeLazyMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GroupNodeLazyMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LayerNodeLazyMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeQueryValues.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_NodeVariableStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PatchNodeLazyMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_WorldNodeLazyMap.cpp
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
