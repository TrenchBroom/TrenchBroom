add_executable(TbGlLibTest)
EMBED_UTF8_MANIFEST(TbGlLibTest)

target_sources(TbGlLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ActiveShader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_IndexRangeMap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Material.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MaterialCollection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MaterialManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MockGl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_OrthographicCamera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PerspectiveCamera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ShaderManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ShaderProgram.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ResourceManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Texture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_TextureBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_VboManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Vbo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Vertex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_VertexArray.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbGlLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbBaseTestUtilsLib
    TbGlLib
    TbGlTestUtilsLib
)

include(Catch)
catch_discover_tests(TbGlLibTest)
