add_executable(TbGlLibTest)

target_sources(TbGlLibTest PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PerspectiveCamera.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ResourceManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Vertex.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbGlLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbGlLibTest PRIVATE Catch2::Catch2WithMain TbBaseTestUtilsLib TbGlLib TbGlTestUtilsLib)
