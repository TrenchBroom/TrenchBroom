ADD_EXECUTABLE(TrenchBroom WIN32 MACOSX_BUNDLE ${SOURCE})
TARGET_LINK_LIBRARIES(TrenchBroom ${wxWidgets_LIBRARIES} glew ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

# Properly link to OpenGL libraries on Linux
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    FIND_PACKAGE(OpenGL)
    TARGET_LINK_LIBRARIES(TrenchBroom ${OPENGL_LIBRARIES})
ENDIF()

# Copy some Windows-specific resources
IF(WIN32)
	# Copy Windows icons to target dir
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/graphics/icons/TrenchBroom.ico" $CMAKE_CURRENT_BINARY_DIR
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/graphics/icons/TrenchBroomDoc.ico" $CMAKE_CURRENT_BINARY_DIR
	)
	
    # Copy DLLs to app directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" $<TARGET_FILE_DIR:TrenchBroom>
	)
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom-Test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" $<TARGET_FILE_DIR:TrenchBroom-Test>
	)

    # Set up resource compilation for Windows
    IF(MSVC)
        SET(SOURCE ${SOURCE} "${SOURCE_DIR}/TrenchBroom.rc")
    ELSEIF(MINGW)
        SET(CMAKE_RC_COMPILER_INIT windres)
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -i <SOURCE> -o <OBJECT>")
    ENDIF()
ENDIF()

# Set up the resources and DLLs for the executable
IF(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Copy button images to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/graphics/images" $<TARGET_FILE_DIR:TrenchBroom>/Resources/images
	)

	# Copy game files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/games/" $<TARGET_FILE_DIR:TrenchBroom>/Resources/games
	)

	# Copy shader files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/shader" $<TARGET_FILE_DIR:TrenchBroom>/Resources/shader
	)
ENDIF()

# OS X app bundle configuration
IF(APPLE)
    # Configure plist file
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/mac/TrenchBroom-Info.plist")

	# Configure icons
	SET(MACOSX_ICON_FILES "${CMAKE_SOURCE_DIR}/resources/graphics/icons/AppIcon.icns" "${CMAKE_SOURCE_DIR}/resources/graphics/icons/DocIcon.icns")
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	SET(SOURCE ${SOURCE} ${MACOSX_ICON_FILES})

	# Configure button bitmaps etc.
	FILE(GLOB_RECURSE MACOSX_IMAGE_FILES
	    "${CMAKE_SOURCE_DIR}/resources/graphics/images/*.png"
	)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/images)
	SET(SOURCE ${SOURCE} ${MACOSX_IMAGE_FILES})

	# Configure game resources
	# Collect all game resources
	FILE(GLOB_RECURSE MACOSX_QUAKE_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Quake/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_QUAKE2_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Quake2/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_HEXEN2_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/Hexen2/*.*"
	)
	FILE(GLOB_RECURSE MACOSX_GAME_CONFIG_FILES
	    "${CMAKE_SOURCE_DIR}/resources/games/*.cfg"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake2)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_HEXEN2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Hexen2)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_GAME_CONFIG_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games)
	SET(SOURCE ${SOURCE} ${MACOSX_QUAKE_FILES} ${MACOSX_QUAKE2_FILES} ${MACOSX_HEXEN2_FILES} ${MACOSX_GAME_CONFIG_FILES})

	# Configure shaders
	# Collect all shaders
	FILE(GLOB_RECURSE MACOSX_SHADER_FILES
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.fragsh"
	    "${CMAKE_SOURCE_DIR}/resources/shader/*.vertsh"
	)

	SET_SOURCE_FILES_PROPERTIES(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
	SET(SOURCE ${SOURCE} ${MACOSX_SHADER_FILES})
ENDIF()

# Configure the XCode generator project
IF(APPLE)
    SET_XCODE_ATTRIBUTES(TrenchBroom)
ENDIF()


