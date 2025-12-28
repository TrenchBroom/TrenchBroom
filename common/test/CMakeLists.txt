set(COMMON_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(COMMON_TEST_UTILS_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/catch/CatchConfig.h"
        "${COMMON_TEST_SOURCE_DIR}/catch/StringMakers.cpp"
        "${COMMON_TEST_SOURCE_DIR}/catch/StringMakers.h"
        "${COMMON_TEST_SOURCE_DIR}/RunAllTests.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/MapDocumentFixture.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/MapDocumentFixture.h"
)

set(COMMON_TEST_SOURCE
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ActionContext.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_Actions.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ClipTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ClipToolController.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_CompilationRunner.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_EntityPropertyModel.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ExtrudeTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_HandleDragTracker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_InputEvent.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_LaunchGameEngine.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_MapDocument.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_MoveHandleDragTracker.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_QPathUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_QPreferenceStore.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_QPreferenceStoreUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_QtUtils.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_RecentDocuments.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_RotateTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ScaleTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_SelectionTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_ShearTool.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_SystemPaths.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_TextOutputAdapter.cpp"
        "${COMMON_TEST_SOURCE_DIR}/ui/tst_UpdateVersion.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

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
target_link_libraries(common PRIVATE CompilerConfig PrecompileStdHeaders PrecompileQtHeaders)
target_include_directories(common-test-utils PUBLIC ${COMMON_TEST_SOURCE_DIR})
target_link_libraries(common-test-utils PRIVATE common TbBaseTestUtilsLib TbFsTestUtilsLib TbMdlTestUtilsLib Catch2::Catch2 Qt6::Test)

macro(configure_test_target TARGET)
    target_link_libraries(${TARGET} PRIVATE common common-test-utils TbBaseTestUtilsLib TbElTestUtilsLib TbFsTestUtilsLib TbMdlTestUtilsLib Catch2::Catch2 UpdateLib fmt::fmt-header-only)
    set_target_properties(${TARGET} PROPERTIES AUTOMOC TRUE)

    target_link_libraries(${TARGET} PRIVATE CompilerConfig)

    # By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
    # but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
    set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>")
endmacro()

add_subdirectory(cmd-tool)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/CmdTool.h"
  CONTENT "#define CMD_TOOL_PATH \"$<TARGET_FILE:cmd-tool>\"")

add_executable(common-test ${COMMON_TEST_SOURCE})
target_include_directories(common-test PRIVATE ${COMMON_TEST_SOURCE_DIR})
target_include_directories(common-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
configure_test_target(common-test)
add_dependencies(common-test cmd-tool)

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
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GLEW::GLEW>" "${TEST_RESOURCE_DEST_DIR}")
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
