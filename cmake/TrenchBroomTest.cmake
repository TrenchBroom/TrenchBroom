SET(TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/test/src")

FILE(GLOB_RECURSE TEST_SOURCE
    "${TEST_SOURCE_DIR}/*.h"
    "${TEST_SOURCE_DIR}/*.cpp"
)

IF(NOT CMAKE_GENERATOR STREQUAL "Xcode")
	ADD_EXECUTABLE(TrenchBroom-Test ${TEST_SOURCE} $<TARGET_OBJECTS:common>)
ELSE()
	# OBJECT libraries are broken on Xcode, so just compile the COMMON source directly in
    # see: https://github.com/kduske/TrenchBroom/issues/1373
	ADD_EXECUTABLE(TrenchBroom-Test ${TEST_SOURCE} ${COMMON_SOURCE} ${COMMON_HEADER})
ENDIF()

ADD_TARGET_PROPERTY(TrenchBroom-Test INCLUDE_DIRECTORIES "${TEST_SOURCE_DIR}")
TARGET_LINK_LIBRARIES(TrenchBroom-Test gtest gmock ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
IF (COMPILER_IS_MSVC)
    TARGET_LINK_LIBRARIES(TrenchBroom-Test stackwalker)
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

