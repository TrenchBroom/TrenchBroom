SET(TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/test/src")
SET(BENCHMARK_SOURCE_DIR "${CMAKE_SOURCE_DIR}/benchmark/src")

FILE(GLOB_RECURSE TEST_SOURCE
    "${TEST_SOURCE_DIR}/*.h"
    "${TEST_SOURCE_DIR}/*.cpp"
)
FILE(GLOB_RECURSE BENCHMARK_SOURCE
    "${BENCHMARK_SOURCE_DIR}/*.h"
    "${BENCHMARK_SOURCE_DIR}/*.cpp"
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
