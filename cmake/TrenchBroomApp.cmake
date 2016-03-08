INCLUDE(cmake/GenerateHelp.cmake)

SET(APP_DIR "${CMAKE_SOURCE_DIR}/app")
SET(APP_SOURCE_DIR "${APP_DIR}/src")

# Collect the source files for compilation.
FILE(GLOB_RECURSE APP_SOURCE
    "${APP_SOURCE_DIR}/*.h"
    "${APP_SOURCE_DIR}/*.cpp"
)

SET(APP_SOURCE ${APP_SOURCE} ${DOC_HELP_TARGET_FILES})

# OS X app bundle configuration, must happen before the executable is added
IF(APPLE)
	# Configure icons
    SET(MACOSX_ICON_FILES "${APP_DIR}/resources/graphics/icons/AppIcon.icns" "${APP_DIR}/resources/graphics/icons/DocIcon.icns")
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_ICON_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	# Configure button bitmaps etc.
	FILE(GLOB_RECURSE MACOSX_IMAGE_FILES
        "${APP_DIR}/resources/graphics/images/*.png"
	)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_IMAGE_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/images)

    FILE(GLOB_RECURSE MACOSX_FONT_FILES
        "${APP_DIR}/resources/fonts/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_FONT_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/fonts)

	# Configure game resources
	# Collect all game resources
	FILE(GLOB_RECURSE MACOSX_QUAKE_FILES
        "${APP_DIR}/resources/games/Quake/*.*"
	)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_QUAKE_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake)

	FILE(GLOB_RECURSE MACOSX_QUAKE2_FILES
        "${APP_DIR}/resources/games/Quake2/*.*"
	)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_QUAKE2_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake2)

	FILE(GLOB_RECURSE MACOSX_HEXEN2_FILES
        "${APP_DIR}/resources/games/Hexen2/*.*"
	)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_HEXEN2_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_HEXEN2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Hexen2)

	FILE(GLOB_RECURSE MACOSX_GAME_CONFIG_FILES
        "${APP_DIR}/resources/games/*.cfg"
	)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_GAME_CONFIG_FILES})
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_GAME_CONFIG_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games)

	# Configure shaders
	# Collect all shaders
	FILE(GLOB_RECURSE MACOSX_SHADER_FILES
        "${APP_DIR}/resources/shader/*.fragsh"
        "${APP_DIR}/resources/shader/*.vertsh"
	)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_SHADER_FILES})

    # Configure help files
    SET_SOURCE_FILES_PROPERTIES(${DOC_HELP_TARGET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/help)
ENDIF()

# Set up resource compilation for Windows
IF(WIN32)
    # CONFIGURE_FILE("${APP_SOURCE_DIR}/TrenchBroom.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/TrenchBroom.rc" @ONLY)
    IF(MSVC)
        SET(APP_SOURCE ${APP_SOURCE} "${APP_SOURCE_DIR}/TrenchBroom.rc")
    ELSEIF(MINGW)
        SET(CMAKE_RC_COMPILER_INIT windres)
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -i <SOURCE> -o <OBJECT>")
    ENDIF()
ENDIF()

ADD_EXECUTABLE(TrenchBroom WIN32 MACOSX_BUNDLE ${APP_SOURCE} $<TARGET_OBJECTS:common>)

TARGET_LINK_LIBRARIES(TrenchBroom glew ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES})
SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

GET_APP_VERSION("${APP_DIR}" CPACK_PACKAGE_VERSION_MAJOR CPACK_PACKAGE_VERSION_MINOR CPACK_PACKAGE_VERSION_PATCH)
GET_BUILD_ID("${GIT_EXECUTABLE}" "${CMAKE_SOURCE_DIR}" APP_BUILD_ID)
GET_BUILD_PLATFORM(APP_PLATFORM_NAME)
IF(NOT DEFINED APP_BUILD_CHANNEL)
    SET(APP_BUILD_CHANNEL "Interim")
ENDIF()

# Create the cmake script for generating the version information
FIND_PACKAGE(Git)
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake" @ONLY)
ADD_TARGET_PROPERTY(TrenchBroom INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(GenerateVersion 
    ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake")
ADD_DEPENDENCIES(TrenchBroom GenerateVersion)

ADD_DEPENDENCIES(TrenchBroom GenerateHelp)

IF(APPLE)
    # Configure variables that are substituted into the plist
    # Set CFBundleExecutable
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}")
    # Set CFBundleName, which controls the application menu label
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "TrenchBroom")
    # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    # Set CFBundleVersion to the git revision. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${APP_BUILD_ID}")

    # Set the path to the plist template
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${APP_DIR}/resources/mac/TrenchBroom-Info.plist")

    # Configure the XCode generator project
    SET_XCODE_ATTRIBUTES(TrenchBroom)
ENDIF()

# Copy some Windows-specific resources
IF(WIN32)
	# Copy Windows icons to target dir
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/graphics/icons/TrenchBroom.ico" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/graphics/icons/TrenchBroomDoc.ico" "${CMAKE_CURRENT_BINARY_DIR}"
	)

    # Copy DLLs to app directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "$<TARGET_FILE_DIR:TrenchBroom>"
	)
ENDIF()

# Properly link to OpenGL libraries on Linux
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    FIND_PACKAGE(OpenGL)
    TARGET_LINK_LIBRARIES(TrenchBroom ${OPENGL_LIBRARIES})
ENDIF()

# Set up the resources and DLLs for the executable
IF(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Copy button images to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/graphics/images" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/images"
	)

    # Copy fonts to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/fonts" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/fonts"
    )

	# Copy game files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/games/" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/games"
	)

	# Copy shader files to resources directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/shader" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/shader"
	)

    # Copy help files to resource directory
	ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TrenchBroom>/Resources/help/"
	)

    FOREACH(HELP_FILE ${DOC_HELP_TARGET_FILES})
        ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${HELP_FILE} "$<TARGET_FILE_DIR:TrenchBroom>/Resources/help/"
        )
    ENDFOREACH(HELP_FILE)
ENDIF()

# Common CPack configuration
SET(APP_PACKAGE_FILE_NAME "TrenchBroom-${APP_PLATFORM_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${APP_BUILD_CHANNEL}-${APP_BUILD_ID}-${CMAKE_BUILD_TYPE}")
SET(APP_PACKAGE_DIR_NAME "$ENV{DROPBOX}/TrenchBroom/")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TrenchBroom Level Editor")
SET(CPACK_PACKAGE_VENDOR "Kristian Duske")

# Platform specific CPack configuration
IF(WIN32)
    IF(MSVC)
        # SET(CMAKE_INSTALLL_DEBUG_LIBRARIES OFF)
        # INCLUDE(InstallRequiredSystemLibraries)
    ENDIF(MSVC)

    FILE(GLOB WIN_LIBS "${LIB_BIN_DIR}/win32/*.dll")
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        LIB_TO_DLL(${WX_cored} _vc100 WIN_LIB_WX_core)
        LIB_TO_DLL(${WX_based} _vc100 WIN_LIB_WX_base)
        LIB_TO_DLL(${WX_advd}  _vc100 WIN_LIB_WX_adv)
        LIB_TO_DLL(${WX_gld}   _vc100 WIN_LIB_WX_gl)
    ELSE()
        LIB_TO_DLL(${WX_core} _vc100 WIN_LIB_WX_core)
        LIB_TO_DLL(${WX_base} _vc100 WIN_LIB_WX_base)
        LIB_TO_DLL(${WX_adv}  _vc100 WIN_LIB_WX_adv)
        LIB_TO_DLL(${WX_gl}   _vc100 WIN_LIB_WX_gl)
    ENDIF()

    INSTALL(TARGETS TrenchBroom RUNTIME DESTINATION . COMPONENT TrenchBroom)
    INSTALL(FILES
        ${WIN_LIB_WX_core}
        ${WIN_LIB_WX_base}
        ${WIN_LIB_WX_adv}
        ${WIN_LIB_WX_gl}
        DESTINATION . COMPONENT TrenchBroom)
    INSTALL(FILES
        ${WIN_LIBS}
        DESTINATION . COMPONENT TrenchBroom)
    INSTALL(FILES
        ${DOC_HELP_TARGET_FILES}
        DESTINATION Resources/help COMPONENT TrenchBroom)
    INSTALL(DIRECTORY
        "${APP_DIR}/resources/graphics/images"
        "${APP_DIR}/resources/games"
        "${APP_DIR}/resources/shader"
        DESTINATION Resources COMPONENT TrenchBroom)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
ELSEIF(APPLE)
    INSTALL(TARGETS TrenchBroom BUNDLE DESTINATION . COMPONENT TrenchBroom)
    SET(CPACK_GENERATOR "DragNDrop")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # add architecture to filename
    SET(APP_PACKAGE_FILE_NAME "${APP_PACKAGE_FILE_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
    SET(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})

    # generate deb and rpm packages
    SET(CPACK_GENERATOR "DEB;RPM")

    # the software will get installed under /opt
    SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt")
    SET(LINUX_TARGET_DIRECTORY "${CPACK_PACKAGING_INSTALL_PREFIX}/trenchbroom")
    SET(LINUX_TARGET_EXECUTABLE_PATH "${LINUX_TARGET_DIRECTORY}/TrenchBroom")

    # configure install scripts
    CONFIGURE_FILE(${APP_DIR}/resources/linux/postinst ${CMAKE_CURRENT_BINARY_DIR}/linux/postinst @ONLY)
    CONFIGURE_FILE(${APP_DIR}/resources/linux/prerm ${CMAKE_CURRENT_BINARY_DIR}/linux/prerm @ONLY)
    CONFIGURE_FILE(${APP_DIR}/resources/linux/postrm ${CMAKE_CURRENT_BINARY_DIR}/linux/postrm @ONLY)

    # add files
    INSTALL(TARGETS TrenchBroom RUNTIME DESTINATION trenchbroom COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources" DESTINATION trenchbroom COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${APP_DIR}/resources/linux/icons" DESTINATION trenchbroom COMPONENT TrenchBroom FILES_MATCHING PATTERN "*.png")
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/gpl.txt" DESTINATION trenchbroom COMPONENT TrenchBroom)
    INSTALL(FILES "${APP_DIR}/resources/linux/copyright" DESTINATION trenchbroom COMPONENT TrenchBroom)
    INSTALL(FILES "${APP_DIR}/resources/linux/trenchbroom.desktop" DESTINATION trenchbroom COMPONENT TrenchBroom)

    # deb package specifics
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
    SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
    SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kristianduske.com/trenchbroom/")
    SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/linux/postinst;${CMAKE_CURRENT_BINARY_DIR}/linux/prerm;${CMAKE_CURRENT_BINARY_DIR}/linux/postrm")

    # rpm package specifics
    SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    SET(CPACK_RPM_PACKAGE_GROUP "Applications/Editors")
    SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    SET(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    SET(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/postinst")
    SET(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/prerm")
    SET(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/postrm")
ENDIF()
INCLUDE(CPack)

IF(WIN32)
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/publish.bat.in ${CMAKE_CURRENT_BINARY_DIR}/publish.bat @ONLY)
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/upload.bat.in ${CMAKE_CURRENT_BINARY_DIR}/upload.bat @ONLY)
ELSE()
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/publish.sh.in ${CMAKE_CURRENT_BINARY_DIR}/publish.sh @ONLY)
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/upload.sh.in ${CMAKE_CURRENT_BINARY_DIR}/upload.sh @ONLY)
ENDIF()

