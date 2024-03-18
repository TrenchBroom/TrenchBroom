set(COMMON_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(COMMON_TEST_UTILS_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.h"
        "${COMMON_TEST_SOURCE_DIR}/Catch2.h"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/Matchers.cpp"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/Matchers.h"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/StringMakers.cpp"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/StringMakers.h"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestEnvironment.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestEnvironment.h"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestFileSystem.h"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestParserStatus.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestParserStatus.h"
        "${COMMON_TEST_SOURCE_DIR}/QtPrettyPrinters.h"
        "${COMMON_TEST_SOURCE_DIR}/RunAllTests.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestLogger.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestPreferenceManager.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestPreferenceManager.h"
        "${COMMON_TEST_SOURCE_DIR}/TestUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestUtils.h"
)

set(COMMON_TEST_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_AssetUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_DecalDefinition.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_EntityModel.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_ModelDefinition.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_Palette.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/tst_Resource.cpp"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/tst_Matchers.cpp"
        "${COMMON_TEST_SOURCE_DIR}/CatchUtils/tst_StringMakers.cpp"
        "${COMMON_TEST_SOURCE_DIR}/EL/tst_EL.cpp"
        "${COMMON_TEST_SOURCE_DIR}/EL/tst_Expression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/EL/tst_Interpolator.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_AseParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_AssimpParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_CompilationConfigParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_DefParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_DiskFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_DiskIO.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ELParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_EntityDefinitionParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_EntParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_FgdParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_FileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_GameConfigParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_GameEngineConfigParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ImageFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_LoadTextureCollection.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Md3Parser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_MdlParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_NodeReader.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_NodeWriter.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ObjSerializer.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Quake3ShaderFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Quake3ShaderParser.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadDdsTexture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Reader.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadFreeImageTexture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadM8Texture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadMipTexture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadQuake3ShaderTexture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ReadWalTexture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_ResourceUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_SystemPaths.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_TestFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_TextureUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Tokenizer.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_VirtualFileSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_WorldReader.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/TestGame.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/TestGame.h"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BezierPatch.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Brush.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BrushBuilder.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BrushFace.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BrushNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EditorContext.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Entity.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EntityNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EntityNodeIndex.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EntityNodeLink.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EntityRotation.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Game.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_GameFactory.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Group.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_GroupNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Issue.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_LayerNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_LinkedGroupUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_ModelUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Node.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_NodeCollection.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_NodeQueries.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_PatchNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_PointTrace.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Polyhedron.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_PortalFile.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_Tagging.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_TexCoordSystem.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_WorldNode.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/tst_AllocationTracker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/tst_Camera.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/tst_Vertex.cpp"
        "${COMMON_TEST_SOURCE_DIR}/tst_Ensure.cpp"
        "${COMMON_TEST_SOURCE_DIR}/tst_Notifier.cpp"
        "${COMMON_TEST_SOURCE_DIR}/tst_octree.cpp"
        "${COMMON_TEST_SOURCE_DIR}/tst_Preferences.cpp"
        "${COMMON_TEST_SOURCE_DIR}/tst_StackWalker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/MapDocumentTest.h"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ActionContext.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_AddNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Autosaver.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ChangeBrushFaceAttributes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ClipTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ClipToolController.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_CommandProcessor.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_CompilationRunner.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_CopyPaste.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Csg.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ExtrudeTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Grid.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_GroupNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_HandleDragTracker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_InputEvent.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_LayerNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_MapDocument.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_MoveHandleDragTracker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Picking.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_RecentDocuments.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_RemoveNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ReparentNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_RepeatableActions.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_ScaleObjectsTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Selection.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SelectionTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SetEntityProperties.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SetLockState.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SetVisibilityState.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SnapBrushVertices.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_SwapNodeContents.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_TagManagement.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_TextOutputAdapter.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Transaction.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_TransformNodes.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Undo.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_UpdateLinkedGroupsCommand.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_UpdateLinkedGroupsHelper.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/tst_Validator.cpp"
)

set(COMMON_REGRESSION_TEST_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_AseParserRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Md3ParserRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_Quake3ShaderParserRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/tst_WorldReaderRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BrushNodeRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_BrushRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_EntityRegression.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/tst_PolyhedronRegression.cpp"
)


# Catch2 needs one source file to define a macro (in our case CATCH_CONFIG_RUNNER) before including
# catch.hpp. This needs to be excluded from unity builds, otherwise the macro wouldn't be defined
# before the first catch.hpp inclusion.
set_property(SOURCE "${COMMON_TEST_SOURCE_DIR}/RunAllTests.cpp" PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

# Organize files into IDE folders
source_group(TREE "${COMMON_TEST_SOURCE_DIR}" FILES ${COMMON_TEST_UTILS_SOURCE})
source_group(TREE "${COMMON_TEST_SOURCE_DIR}" FILES ${COMMON_TEST_SOURCE})

# Configure test fixture directories
set(TEST_FIXTURE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixture")

add_library(common-test-utils OBJECT ${COMMON_TEST_UTILS_SOURCE})
set_compiler_config(common-test-utils)
target_include_directories(common-test-utils PUBLIC ${COMMON_TEST_SOURCE_DIR})
target_link_libraries(common-test-utils PRIVATE common Catch2::Catch2 Qt5::Test)

macro(configure_test_target TARGET)
    target_link_libraries(${TARGET} PRIVATE common common-test-utils Catch2::Catch2)
    set_target_properties(${TARGET} PROPERTIES AUTOMOC TRUE)

    set_compiler_config(${TARGET})

    # By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
    # but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
    set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>")
endmacro()

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/ReturnExitCode.h"
  CONTENT "#define RETURN_EXITCODE_PATH \"$<TARGET_FILE:return-exitcode>\"")

add_executable(common-test ${COMMON_TEST_SOURCE})
target_include_directories(common-test PRIVATE ${COMMON_TEST_SOURCE_DIR})
target_include_directories(common-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
configure_test_target(common-test)
add_dependencies(common-test return-exitcode)

add_executable(common-regression-test ${COMMON_REGRESSION_TEST_SOURCE})
target_include_directories(common-regression-test PRIVATE ${COMMON_TEST_SOURCE_DIR})
configure_test_target(common-regression-test)

# Copy shared resources for the test executables
# We only need to do this once and not for both targets
set(TEST_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:common-test>")
set(TEST_FIXTURE_DEST_DIR "${TEST_RESOURCE_DEST_DIR}/fixture")

if(WIN32)
    # Copy DLLs to app directory
    add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:assimp::assimp>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage::FreeImage>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:tinyxml2::tinyxml2>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:miniz::miniz>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:fmt::fmt>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GLEW::GLEW>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Widgets>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Gui>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Core>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Svg>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Test>" "${TEST_RESOURCE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "${TEST_RESOURCE_DEST_DIR}/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "${TEST_RESOURCE_DEST_DIR}/styles"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" "${TEST_RESOURCE_DEST_DIR}/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" "${TEST_RESOURCE_DEST_DIR}/styles")
endif()

# Copy some resource files required when initializing TrenchBroomApp
add_custom_command(TARGET common-test POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/graphics/images" "${TEST_RESOURCE_DEST_DIR}/images")

# Copy fixtures
add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games" "${TEST_FIXTURE_DEST_DIR}/games"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games-testing" "${TEST_FIXTURE_DEST_DIR}/games")
