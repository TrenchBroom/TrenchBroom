add_executable(TbUiLibTest)

target_sources(TbUiLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/RunAllTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ActionContext.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Actions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ClipTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ClipToolController.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_CompilationRunner.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_EntityPropertyModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ExtrudeTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_HandleDragTracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_InputEvent.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_LaunchGameEngine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MapDocument.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_MoveHandleDragTracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_QPathUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_QPreferenceStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_QPreferenceStoreUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_QVecUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_RecentDocuments.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_RotateTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ScaleTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SelectionTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ShearTool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_SystemPaths.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_TextOutputAdapter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UpdateVersion.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbUiLibTest PRIVATE 
  CompilerConfig
  PrecompileQtHeaders
  PrecompileStdHeaders
)

target_link_libraries(TbUiLibTest PRIVATE 
  Catch2::Catch2WithMain
  Qt6::Network
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  Qt6::Svg
  Qt6::Test
  Qt6::Widgets
  TbBaseTestUtilsLib
  TbElLib
  TbElTestUtilsLib
  TbFsTestUtilsLib
  TbMdlTestUtilsLib
  TbPreferencesLib
  TbRenderLib
  TbUiLib
  TbUiTestUtilsLib
  UpdateLib
)

set_target_properties(TbUiLibTest PROPERTIES AUTOMOC TRUE)

add_dependencies(TbUiLibTest CmdTool)
target_include_directories(TbUiLibTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/CmdTool.h"
  CONTENT "#define CMD_TOOL_PATH \"$<TARGET_FILE:CmdTool>\"")

# Copy fixtures
set(TEST_FIXTURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixture)
set(TEST_FIXTURE_DEST_DIR $<TARGET_FILE_DIR:TbUiLibTest>/fixture)

add_custom_command(TARGET TbUiLibTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games" "${TEST_FIXTURE_DEST_DIR}/games"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TB_RESOURCE_DIR}/games-testing" "${TEST_FIXTURE_DEST_DIR}/games")
