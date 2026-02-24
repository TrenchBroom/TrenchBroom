cmake_minimum_required(VERSION 3.12)

add_library(TbGlLib STATIC)

target_sources(TbGlLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ActiveShader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AttrString.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContextManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontDescriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontGlyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontGlyphBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FontTexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FreeTypeFontFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GlManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexArray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexArrayMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexArrayMapBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexRangeMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexRangeRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialCollection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialIndexArrayMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialIndexArrayMapBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialIndexArrayRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialIndexRangeMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialIndexRangeRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialRenderFunc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OrthographicCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PerspectiveCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PrimType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceId.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShaderManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShaderProgram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextureBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextureFont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextureResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vbo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VboManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VertexArray.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TbGlLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbGlLib PRIVATE GLEW::GLEW OpenGL::GL freetype)
target_link_libraries(TbGlLib PUBLIC TbBaseLib TbFsLib KdLib VmLib fmt::fmt-header-only)

add_subdirectory(test)
add_subdirectory(test-utils)
