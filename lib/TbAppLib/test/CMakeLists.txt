add_executable(TbAppLibTest)
EMBED_UTF8_MANIFEST(TbAppLibTest)

target_sources(TbAppLibTest
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RunAllTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_CameraTool3D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ClipTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ClipToolController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ControlPointTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DrawShapeToolExtensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ExtrudeTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ExtrudeToolController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_HandleDragTracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MapDocument.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MoveHandleDragTracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_RotateTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ScaleTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SelectionTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SetBrushFaceAttributesTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ShearTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SweepTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SweepToolUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ToolChain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvOffsetTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvOriginTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvRotateTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvScaleTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvShearTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UvViewHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_VertexTool.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbAppLibTest
  PRIVATE
    CompilerConfig
    PrecompileStdHeaders
    Catch2::Catch2WithMain
    TbAppLib
    TbAppTestUtilsLib
    TbBaseTestUtilsLib
    TbMdlTestUtilsLib
)

# Copy fixtures
set(TEST_FIXTURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixture)
set(TEST_FIXTURE_DEST_DIR $<TARGET_FILE_DIR:TbAppLibTest>/fixture)

add_custom_command(TARGET TbAppLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games" "${TEST_FIXTURE_DEST_DIR}/games"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games-testing" "${TEST_FIXTURE_DEST_DIR}/games"
)

include(Catch)
catch_discover_tests(TbAppLibTest)
