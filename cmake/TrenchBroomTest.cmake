SET(TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/test/src")

FILE(GLOB_RECURSE TEST_SOURCE
    "${TEST_SOURCE_DIR}/*.h"
    "${TEST_SOURCE_DIR}/*.cpp"
)

ADD_EXECUTABLE(TrenchBroom-Test ${TEST_SOURCE} $<TARGET_OBJECTS:common>)

ADD_TARGET_PROPERTY(TrenchBroom-Test INCLUDE_DIRECTORIES "${TEST_SOURCE_DIR}")
TARGET_LINK_LIBRARIES(TrenchBroom-Test gtest gmock ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
IF (COMPILER_IS_MSVC)
    TARGET_LINK_LIBRARIES(TrenchBroom-Test stackwalker)
    # Generate a small stripped PDB for release builds so we get stack traces with symbols
    SET_TARGET_PROPERTIES(TrenchBroom-Test PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/TrenchBroom-Test-stripped.pdb /PDBALTPATH:TrenchBroom-Test-stripped.pdb")
ENDIF()

SET(RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:TrenchBroom-Test>")

IF(WIN32)
	SET(RESOURCE_DEST_DIR "${RESOURCE_DEST_DIR}/..")
	
	# Copy some Windows-specific resources
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "${RESOURCE_DEST_DIR}"
	)
ENDIF()

# Copy some files used in unit tests
ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test/data" "${RESOURCE_DEST_DIR}/data"
)

ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DEST_DIR}/data/GameConfig"
)

# Prepare to collect all cfg files to copy them to the test data
FILE(GLOB_RECURSE GAME_CONFIG_FILES
    "${APP_DIR}/resources/games/*.cfg"
)

FOREACH(GAME_CONFIG_FILE ${GAME_CONFIG_FILES})
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${GAME_CONFIG_FILE}" "${RESOURCE_DEST_DIR}/data/GameConfig"
    )
ENDFOREACH(GAME_CONFIG_FILE)

SET_XCODE_ATTRIBUTES(TrenchBroom-Test)

