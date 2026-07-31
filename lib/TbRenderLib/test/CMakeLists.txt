add_executable(TbRenderLibTest)
EMBED_UTF8_MANIFEST(TbRenderLibTest)

target_sources(TbRenderLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_AllocationTracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BoundsGuideRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_BrushRendererArrays.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Circle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GridRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_PointGuideRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_RenderBatch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_RenderContext.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbRenderLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbGlTestUtilsLib
    TbRenderLib
  )

include(Catch)
catch_discover_tests(TbRenderLibTest)
