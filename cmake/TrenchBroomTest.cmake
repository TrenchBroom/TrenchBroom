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

IF(WIN32)
	# Copy some Windows-specific resources
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "$<TARGET_FILE_DIR:TrenchBroom-Test>/../"
	)
	
	# Copy some files used in unit tests
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test/data" "$<TARGET_FILE_DIR:TrenchBroom-Test>/../data"
	)
ELSE()
	# Copy some files used in unit tests
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/test/data" "$<TARGET_FILE_DIR:TrenchBroom-Test>/data"
	)
ENDIF()

SET_XCODE_ATTRIBUTES(TrenchBroom-Test)

