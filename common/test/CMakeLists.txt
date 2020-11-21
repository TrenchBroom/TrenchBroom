set(COMMON_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_TEST_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/Assets/AssetUtilsTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.h"
        "${COMMON_TEST_SOURCE_DIR}/EL/ELTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/EL/ExpressionTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/EL/InterpolatorTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/AseParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/CompilationConfigParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/DefParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/DiskFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/DkPakFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/ELParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/EntParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/EntityDefinitionParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/EntityModelTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/FgdParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/FreeImageTextureReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/GameConfigParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/GameEngineConfigParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/IdMipTextureReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/IdPakFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/M8TextureReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/Md3ParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/MdlParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/NodeWriterTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/ObjParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/PathTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/PathSuffixNameStrategyTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/Quake3ShaderFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/Quake3ShaderParserTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/ReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/ResourceUtilsTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestEnvironment.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestEnvironment.h"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestParserStatus.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TestParserStatus.h"
        "${COMMON_TEST_SOURCE_DIR}/IO/TextureLoaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/TokenizerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/WadFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/WalTextureReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/WorldReaderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/IO/ZipFileSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/AttributableIndexTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/AttributableLinkTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/BrushBuilderTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/BrushFaceTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/BrushNodeTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/BrushTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/EditorContextTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/EntityNodeTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/EntityRotationPolicyTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/EntityTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/GameTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/NodeTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/PolyhedronTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/PortalFileTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/TaggingTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/TestGame.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Model/TestGame.h"
        "${COMMON_TEST_SOURCE_DIR}/Model/TexCoordSystemTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/AllocationTrackerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/CameraTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Renderer/VertexTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/AutosaverTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ChangeBrushFaceAttributesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ChangeEntityAttributesCommandTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ClipToolControllerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/CommandProcessorTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/CompilationRunToolTaskRunnerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/GridTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/GroupNodesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/InputEventTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/KeyboardShortcutTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/MapDocumentTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/MapDocumentTest.h"
        "${COMMON_TEST_SOURCE_DIR}/View/MoveToolControllerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/RemoveNodesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ResizeBrushesToolTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ReparentNodesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/RepeatableActionsTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/ScaleObjectsToolTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/SelectionCommandTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/SelectionTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/SnapBrushVerticesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/SnapshotTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/TagManagementTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/View/TextOutputAdapterTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/AABBTreeStressTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/AABBTreeTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/Catch2.h"
        "${COMMON_TEST_SOURCE_DIR}/EnsureTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/NotifierTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/PreferencesTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/QtPrettyPrinters.h"
        "${COMMON_TEST_SOURCE_DIR}/RunAllTests.cpp"
        "${COMMON_TEST_SOURCE_DIR}/StackWalkerTest.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestLogger.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/TestUtils.h"
)

# Catch2 needs one source file to define a macro (in our case CATCH_CONFIG_RUNNER) before including
# catch.hpp. This needs to be excluded from unity builds, otherwise the macro wouldn't be defined
# before the first catch.hpp inclusion.
set_property(SOURCE "${COMMON_TEST_SOURCE_DIR}/RunAllTests.cpp" PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

add_executable(common-test ${COMMON_TEST_SOURCE})
target_include_directories(common-test PRIVATE ${COMMON_TEST_SOURCE_DIR})
target_link_libraries(common-test PRIVATE common Catch2::Catch2)

set_compiler_config(common-test)

# avoid spurious warnings when passing 0 to google test assertions
if(COMPILER_IS_CLANG)
    target_compile_options(common-test PRIVATE -Wno-zero-as-null-pointer-constant)
endif()

# By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
# but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
set_target_properties(common-test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:common-test>")

# Organize files into IDE folders
source_group(TREE "${COMMON_TEST_SOURCE_DIR}" FILES ${COMMON_TEST_SOURCE})

set(TEST_FIXTURE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixture")

set(TEST_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:common-test>")
set(TEST_FIXTURE_DEST_DIR "${TEST_RESOURCE_DEST_DIR}/fixture")

if(WIN32)
    # Copy DLLs to app directory
    add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Widgets>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Gui>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Core>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Svg>" "$<TARGET_FILE_DIR:common-test>"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:common-test>/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:common-test>/styles"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" "$<TARGET_FILE_DIR:common-test>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" "$<TARGET_FILE_DIR:common-test>/styles")
endif()

# Copy some resource files required when initializing TrenchBroomApp
add_custom_command(TARGET common-test POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/graphics/images" "${TEST_RESOURCE_DEST_DIR}/images")

# Clear all fixtures
add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${TEST_FIXTURE_DEST_DIR}")

# Copy test fixtures
add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test")

# Copy game config files
add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games" "${TEST_FIXTURE_DEST_DIR}/games")

add_custom_command(TARGET common-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games-testing" "${TEST_FIXTURE_DEST_DIR}/games")

