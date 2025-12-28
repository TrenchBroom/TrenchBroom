cmake_minimum_required(VERSION 3.12)

add_library(TbRenderLib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AllocationTracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BoundsGuideRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BrushRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BrushRendererArrays.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Circle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Compass.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Compass2D.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Compass3D.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EdgeRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityDecalRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityLinkRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityModelRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FaceRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GridRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GroupLinkRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GroupRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MapRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ObjectRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PatchRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PointGuideRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PointHandleRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PrimitiveRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderBatch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderContext.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SelectionBoundsRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Sphere.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SpikeGuideRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TextAnchor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TextRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Transformation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TriangleRenderer.cpp
)

target_include_directories(TbRenderLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TbRenderLib PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbRenderLib PRIVATE KdLib TbBaseLib TbPreferencesLib fmt::fmt-header-only)
target_link_libraries(TbRenderLib PUBLIC VmLib TbGlLib TbMdlLib)

add_subdirectory(test)
