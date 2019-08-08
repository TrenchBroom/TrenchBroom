SET(TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/test/src")
SET(BENCHMARK_SOURCE_DIR "${CMAKE_SOURCE_DIR}/benchmark/src")

SET(TEST_SOURCE
    "${TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.h"
    "${TEST_SOURCE_DIR}/IO/TestEnvironment.h"
    "${TEST_SOURCE_DIR}/IO/TestParserStatus.h"
    "${TEST_SOURCE_DIR}/MockObserver.h"
    "${TEST_SOURCE_DIR}/Model/TestGame.h"
    "${TEST_SOURCE_DIR}/QtPrettyPrinters.h"
    "${TEST_SOURCE_DIR}/TestUtils.h"
    "${TEST_SOURCE_DIR}/View/MapDocumentTest.h"
    "${TEST_SOURCE_DIR}/AABBTreeStressTest.cpp"
    "${TEST_SOURCE_DIR}/AABBTreeTest.cpp"
    "${TEST_SOURCE_DIR}/Assets/EntityDefinitionTestUtils.cpp"
    "${TEST_SOURCE_DIR}/bbox_test.cpp"
    "${TEST_SOURCE_DIR}/CollectionUtilsTest.cpp"
    "${TEST_SOURCE_DIR}/convex_hull_test.cpp"
    "${TEST_SOURCE_DIR}/distance_test.cpp"
    "${TEST_SOURCE_DIR}/DoublyLinkedListTest.cpp"
    "${TEST_SOURCE_DIR}/EL/ELTest.cpp"
    "${TEST_SOURCE_DIR}/EL/ExpressionTest.cpp"
    "${TEST_SOURCE_DIR}/EL/InterpolatorTest.cpp"
    "${TEST_SOURCE_DIR}/EnsureTest.cpp"
    "${TEST_SOURCE_DIR}/intersection_test.cpp"
    "${TEST_SOURCE_DIR}/IO/AseParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/CompilationConfigParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/DefParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/DiskFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/IO/DkPakFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/IO/ELParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/EntParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/FgdParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/FreeImageTextureReaderTest.cpp"
    "${TEST_SOURCE_DIR}/IO/GameConfigParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/IdMipTextureReaderTest.cpp"
    "${TEST_SOURCE_DIR}/IO/IdPakFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/IO/Md3ParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/MdlParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/NodeWriterTest.cpp"
    "${TEST_SOURCE_DIR}/IO/PathTest.cpp"
    "${TEST_SOURCE_DIR}/IO/Quake3ShaderFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/IO/Quake3ShaderParserTest.cpp"
    "${TEST_SOURCE_DIR}/IO/ReaderTest.cpp"
    "${TEST_SOURCE_DIR}/IO/TestEnvironment.cpp"
    "${TEST_SOURCE_DIR}/IO/TestParserStatus.cpp"
    "${TEST_SOURCE_DIR}/IO/TokenizerTest.cpp"
    "${TEST_SOURCE_DIR}/IO/WadFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/IO/WalTextureReaderTest.cpp"
    "${TEST_SOURCE_DIR}/IO/WorldReaderTest.cpp"
    "${TEST_SOURCE_DIR}/IO/ZipFileSystemTest.cpp"
    "${TEST_SOURCE_DIR}/line_test.cpp"
    "${TEST_SOURCE_DIR}/mat_ext_test.cpp"
    "${TEST_SOURCE_DIR}/mat_test.cpp"
    "${TEST_SOURCE_DIR}/Model/AttributableIndexTest.cpp"
    "${TEST_SOURCE_DIR}/Model/AttributableLinkTest.cpp"
    "${TEST_SOURCE_DIR}/Model/BrushBuilderTest.cpp"
    "${TEST_SOURCE_DIR}/Model/BrushFaceTest.cpp"
    "${TEST_SOURCE_DIR}/Model/BrushTest.cpp"
    "${TEST_SOURCE_DIR}/Model/EditorContextTest.cpp"
    "${TEST_SOURCE_DIR}/Model/EntityTest.cpp"
    "${TEST_SOURCE_DIR}/Model/GameTest.cpp"
    "${TEST_SOURCE_DIR}/Model/NodeTest.cpp"
    "${TEST_SOURCE_DIR}/Model/PlanePointFinderTest.cpp"
    "${TEST_SOURCE_DIR}/Model/PortalFileTest.cpp"
    "${TEST_SOURCE_DIR}/Model/TaggingTest.cpp"
    "${TEST_SOURCE_DIR}/Model/TestGame.cpp"
    "${TEST_SOURCE_DIR}/Model/TexCoordSystemTest.cpp"
    "${TEST_SOURCE_DIR}/NotifierTest.cpp"
    "${TEST_SOURCE_DIR}/plane_test.cpp"
    "${TEST_SOURCE_DIR}/polygon_test.cpp"
    "${TEST_SOURCE_DIR}/PolyhedronTest.cpp"
    "${TEST_SOURCE_DIR}/PreferencesTest.cpp"
    "${TEST_SOURCE_DIR}/quat_test.cpp"
    "${TEST_SOURCE_DIR}/ray_test.cpp"
    "${TEST_SOURCE_DIR}/relation_test.cpp"
    "${TEST_SOURCE_DIR}/Renderer/AllocationTrackerTest.cpp"
    "${TEST_SOURCE_DIR}/Renderer/CameraTest.cpp"
    "${TEST_SOURCE_DIR}/Renderer/VertexTest.cpp"
    "${TEST_SOURCE_DIR}/RunAllTests.cpp"
    "${TEST_SOURCE_DIR}/scalar_test.cpp"
    "${TEST_SOURCE_DIR}/segment_test.cpp"
    "${TEST_SOURCE_DIR}/StackWalkerTest.cpp"
    "${TEST_SOURCE_DIR}/StepIteratorTest.cpp"
    "${TEST_SOURCE_DIR}/StringMapTest.cpp"
    "${TEST_SOURCE_DIR}/StringUtilsTest.cpp"
    "${TEST_SOURCE_DIR}/TestUtils.cpp"
    "${TEST_SOURCE_DIR}/vec_test.cpp"
    "${TEST_SOURCE_DIR}/View/AutosaverTest.cpp"
    "${TEST_SOURCE_DIR}/View/ChangeBrushFaceAttributesTest.cpp"
    "${TEST_SOURCE_DIR}/View/ClipToolControllerTest.cpp"
    "${TEST_SOURCE_DIR}/View/GridTest.cpp"
    "${TEST_SOURCE_DIR}/View/GroupNodesTest.cpp"
    "${TEST_SOURCE_DIR}/View/KeyboardShortcutTest.cpp"
    "${TEST_SOURCE_DIR}/View/MapDocumentTest.cpp"
    "${TEST_SOURCE_DIR}/View/MoveToolControllerTest.cpp"
    "${TEST_SOURCE_DIR}/View/RemoveNodesTest.cpp"
    "${TEST_SOURCE_DIR}/View/ReparentNodesTest.cpp"
    "${TEST_SOURCE_DIR}/View/ScaleObjectsToolTest.cpp"
    "${TEST_SOURCE_DIR}/View/SelectionCommandTest.cpp"
    "${TEST_SOURCE_DIR}/View/SelectionTest.cpp"
    "${TEST_SOURCE_DIR}/View/SnapBrushVerticesTest.cpp"
    "${TEST_SOURCE_DIR}/View/SnapshotTest.cpp"
    "${TEST_SOURCE_DIR}/View/TagManagementTest.cpp"
)

SET(BENCHMARK_SOURCE
    "${BENCHMARK_SOURCE_DIR}/BenchmarkUtils.h"
    "${BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.h"
    "${BENCHMARK_SOURCE_DIR}/AABBTreeBenchmark.cpp"
    "${BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.cpp"
    "${BENCHMARK_SOURCE_DIR}/Main.cpp"
    "${BENCHMARK_SOURCE_DIR}/Renderer/BrushRendererBenchmark.cpp"
)

get_target_property(common_TYPE common TYPE)
IF(common_TYPE STREQUAL "OBJECT_LIBRARY")
    ADD_EXECUTABLE(TrenchBroom-Test ${TEST_SOURCE} $<TARGET_OBJECTS:common>)
    ADD_EXECUTABLE(TrenchBroom-Benchmark ${BENCHMARK_SOURCE} $<TARGET_OBJECTS:common>)
ELSE()
    ADD_EXECUTABLE(TrenchBroom-Test ${TEST_SOURCE})
    ADD_EXECUTABLE(TrenchBroom-Benchmark ${BENCHMARK_SOURCE})
    TARGET_LINK_LIBRARIES(TrenchBroom-Test common)
    TARGET_LINK_LIBRARIES(TrenchBroom-Benchmark common)
ENDIF()

IF(COMPILER_IS_GNU AND TB_ENABLE_ASAN)
    TARGET_LINK_LIBRARIES(TrenchBroom-Test asan)
    TARGET_LINK_LIBRARIES(TrenchBroom-Benchmark asan)
ENDIF()

ADD_TARGET_PROPERTY(TrenchBroom-Test INCLUDE_DIRECTORIES "${TEST_SOURCE_DIR}")
ADD_TARGET_PROPERTY(TrenchBroom-Benchmark INCLUDE_DIRECTORIES "${BENCHMARK_SOURCE_DIR}")

TARGET_LINK_LIBRARIES(TrenchBroom-Test glew ${FREEIMAGE_LIBRARIES} ${FREETYPE_LIBRARIES} vecmath Qt5::Widgets tinyxml2 miniz gtest gmock)
TARGET_LINK_LIBRARIES(TrenchBroom-Benchmark glew ${FREEIMAGE_LIBRARIES} ${FREETYPE_LIBRARIES} vecmath Qt5::Widgets tinyxml2 miniz gtest gmock)

# Work around for gtest bug, see: https://stackoverflow.com/questions/21116622/undefined-reference-to-pthread-key-create-linker-error
find_package(Threads)
target_link_libraries(TrenchBroom-Test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(TrenchBroom-Benchmark ${CMAKE_THREAD_LIBS_INIT})

SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")
SET_TARGET_PROPERTIES(TrenchBroom-Benchmark PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES AUTOMOC TRUE)
SET_TARGET_PROPERTIES(TrenchBroom-Benchmark PROPERTIES AUTOMOC TRUE)

IF (COMPILER_IS_MSVC)
    TARGET_LINK_LIBRARIES(TrenchBroom-Test stackwalker)
    TARGET_LINK_LIBRARIES(TrenchBroom-Benchmark stackwalker)

    # Generate a small stripped PDB for release builds so we get stack traces with symbols
    SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/TrenchBroom-Test-stripped.pdb /PDBALTPATH:TrenchBroom-Test-stripped.pdb")
    SET_TARGET_PROPERTIES(TrenchBroom-Benchmark PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/TrenchBroom-Benchmark-stripped.pdb /PDBALTPATH:TrenchBroom-Benchmark-stripped.pdb")
ENDIF()

# Properly link to OpenGL libraries
FIND_PACKAGE(OpenGL REQUIRED)
TARGET_LINK_LIBRARIES(TrenchBroom-Test OpenGL::GL)
TARGET_LINK_LIBRARIES(TrenchBroom-Benchmark OpenGL::GL)

SET(TEST_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:TrenchBroom-Test>")
SET(BENCHMARK_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:TrenchBroom-Benchmark>")

IF(WIN32)
    SET(TEST_RESOURCE_DEST_DIR "${TEST_RESOURCE_DEST_DIR}/..")
    SET(BENCHMARK_RESOURCE_DEST_DIR "${BENCHMARK_RESOURCE_DEST_DIR}/..")

    # Copy some Windows-specific resources
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "${TEST_RESOURCE_DEST_DIR}"
    )
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "${BENCHMARK_RESOURCE_DEST_DIR}"
    )
ENDIF()

SET(TEST_FIXTURE_DEST_DIR "${TEST_RESOURCE_DEST_DIR}/fixture/test")
SET(BENCHMARK_FIXTURE_DEST_DIR "${BENCHMARK_RESOURCE_DEST_DIR}/fixture/benchmark")

# Clear all fixtures
ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${TEST_FIXTURE_DEST_DIR}"
)
ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Benchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BENCHMARK_FIXTURE_DEST_DIR}"
)

# Copy test fixtures
ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test/fixture" "${TEST_FIXTURE_DEST_DIR}"
)

ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/games" "${TEST_FIXTURE_DEST_DIR}/games"
)

ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/games-testing" "${TEST_FIXTURE_DEST_DIR}/games"
)

# Copy benchmark fixtures
ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Benchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/benchmark/fixture" "${BENCHMARK_FIXTURE_DEST_DIR}"
)

SET_XCODE_ATTRIBUTES(TrenchBroom-Test)
SET_XCODE_ATTRIBUTES(TrenchBroom-Benchmark)
