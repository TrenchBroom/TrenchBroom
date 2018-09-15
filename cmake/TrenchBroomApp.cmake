INCLUDE(cmake/GenerateManual.cmake)

SET(APP_DIR "${CMAKE_SOURCE_DIR}/app")
SET(APP_SOURCE_DIR "${APP_DIR}/src")

# Collect the source files for compilation.
FILE(GLOB_RECURSE APP_SOURCE
    "${APP_SOURCE_DIR}/*.h"
    "${APP_SOURCE_DIR}/*.cpp"
)

SET(APP_SOURCE ${APP_SOURCE} ${DOC_MANUAL_TARGET_FILES})

# OS X app bundle configuration, must happen before the executable is added
IF(APPLE)
    # Configure icons
    SET(MACOSX_ICON_FILES "${APP_DIR}/resources/mac/icons/AppIcon.icns" "${APP_DIR}/resources/mac/icons/DocIcon.icns")
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_ICON_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/)

    # Configure button bitmaps etc.
    FILE(GLOB_RECURSE MACOSX_IMAGE_FILES
        "${APP_DIR}/resources/graphics/images/*.png"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_IMAGE_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/images/)

    FILE(GLOB_RECURSE MACOSX_FONT_FILES
        "${APP_DIR}/resources/fonts/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_FONT_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/fonts/)

    # Configure game resources
    # Collect all game resources
    FILE(GLOB_RECURSE MACOSX_QUAKE_FILES
        "${APP_DIR}/resources/games/Quake/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_QUAKE_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake/)

    FILE(GLOB_RECURSE MACOSX_QUAKE2_FILES
        "${APP_DIR}/resources/games/Quake2/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_QUAKE2_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_QUAKE2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Quake2/)

    FILE(GLOB_RECURSE MACOSX_HEXEN2_FILES
        "${APP_DIR}/resources/games/Hexen2/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_HEXEN2_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_HEXEN2_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Hexen2/)

    FILE(GLOB_RECURSE MACOSX_DAIKATANA_FILES
        "${APP_DIR}/resources/games/Daikatana/*.*"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_DAIKATANA_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_DAIKATANA_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/Daikatana/)

    FILE(GLOB_RECURSE MACOSX_GAME_CONFIG_FILES
        "${APP_DIR}/resources/games/*.cfg"
    )
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_GAME_CONFIG_FILES})
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_GAME_CONFIG_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/)

    # Configure shaders
    # Collect all shaders
    FILE(GLOB_RECURSE MACOSX_SHADER_FILES
        "${APP_DIR}/resources/shader/*.fragsh"
        "${APP_DIR}/resources/shader/*.vertsh"
    )
    SET_SOURCE_FILES_PROPERTIES(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader/)
    SET(APP_SOURCE ${APP_SOURCE} ${MACOSX_SHADER_FILES})

    # Configure manual files
    SET_SOURCE_FILES_PROPERTIES(${DOC_MANUAL_TARGET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual/)
ENDIF()

# Set up resource compilation for Windows
IF(WIN32)
    # CONFIGURE_FILE("${APP_SOURCE_DIR}/TrenchBroom.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/TrenchBroom.rc" @ONLY)
    IF(COMPILER_IS_MSVC)
        SET(APP_SOURCE ${APP_SOURCE} "${APP_SOURCE_DIR}/TrenchBroom.rc")
    ELSEIF(MINGW)
        SET(CMAKE_RC_COMPILER_INIT windres)
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -i <SOURCE> -o <OBJECT>")
    ENDIF()
ENDIF()

ADD_EXECUTABLE(TrenchBroom WIN32 MACOSX_BUNDLE ${APP_SOURCE} $<TARGET_OBJECTS:common>)

IF(COMPILER_IS_GNU AND TB_ENABLE_ASAN)
    TARGET_LINK_LIBRARIES(TrenchBroom asan)
ENDIF()

TARGET_LINK_LIBRARIES(TrenchBroom glew ${wxWidgets_LIBRARIES} ${FREETYPE_LIBRARIES} ${FREEIMAGE_LIBRARIES} vecmath)
IF (COMPILER_IS_MSVC)
    TARGET_LINK_LIBRARIES(TrenchBroom stackwalker)
ENDIF()
SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

FIND_PACKAGE(Git)
IF (NOT GIT_FOUND)
    MESSAGE(WARNING "Could not find git")
ENDIF()

GET_GIT_DESCRIBE("${GIT_EXECUTABLE}" "${CMAKE_SOURCE_DIR}" GIT_DESCRIBE)
GET_APP_VERSION(GIT_DESCRIBE CPACK_PACKAGE_VERSION_MAJOR CPACK_PACKAGE_VERSION_MINOR CPACK_PACKAGE_VERSION_PATCH)
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
GET_BUILD_PLATFORM(APP_PLATFORM_NAME)

# Create the cmake script for generating the version information
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake" @ONLY)
ADD_TARGET_PROPERTY(TrenchBroom INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(GenerateVersion 
    ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/GenerateVersion.cmake")
ADD_DEPENDENCIES(TrenchBroom GenerateVersion)

ADD_DEPENDENCIES(TrenchBroom GenerateManual)

IF(APPLE)
    # Configure variables that are substituted into the plist
    # Set CFBundleExecutable
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}")
    # Set CFBundleName, which controls the application menu label
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "TrenchBroom")
    # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    # Set CFBundleVersion to the git describe output. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${GIT_DESCRIBE}")

    # Set the path to the plist template
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${APP_DIR}/resources/mac/TrenchBroom-Info.plist")

    # Configure the XCode generator project
    SET_XCODE_ATTRIBUTES(TrenchBroom)
ENDIF()

# Copy some Windows-specific resources
IF(WIN32)
    # Copy Windows icons to target dir
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/win32/icons/AppIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/win32/icons/DocIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/win32/icons/WindowIcon.ico" "${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Copy DLLs to app directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIB_BIN_DIR}/win32" "$<TARGET_FILE_DIR:TrenchBroom>"
    )

    # Copy application and window icons to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/win32/icons/AppIcon.ico"    "$<TARGET_FILE_DIR:TrenchBroom>/Resources/AppIcon.ico"
        COMMAND ${CMAKE_COMMAND} -E copy "${APP_DIR}/resources/win32/icons/WindowIcon.ico" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/WindowIcon.ico"
    )
ENDIF()

# Generate a small stripped PDB for release builds so we get stack traces with symbols
IF(COMPILER_IS_MSVC)
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/TrenchBroom-stripped.pdb /PDBALTPATH:TrenchBroom-stripped.pdb")
ENDIF()

# Properly link to OpenGL libraries on Unix-like systems
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
    FIND_PACKAGE(OpenGL)
    INCLUDE_DIRECTORIES(SYSTEM ${OPENGL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(TrenchBroom ${OPENGL_LIBRARIES})

    # make executable name conventional lowercase on linux
    SET_TARGET_PROPERTIES(TrenchBroom PROPERTIES OUTPUT_NAME "trenchbroom")
ENDIF()

# Set up the resources and DLLs for the executable
IF(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
    # Copy button images to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/graphics/images" "$<TARGET_FILE_DIR:TrenchBroom>/images"
    )

    # Copy fonts to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/fonts" "$<TARGET_FILE_DIR:TrenchBroom>/fonts"
    )

    # Copy game files to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/games/" "$<TARGET_FILE_DIR:TrenchBroom>/games"
    )

    # Copy shader files to resources directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_DIR}/resources/shader" "$<TARGET_FILE_DIR:TrenchBroom>/shader"
    )

    # Copy manual files to resource directory
    ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TrenchBroom>/manual/"
    )

    FOREACH(MANUAL_FILE ${DOC_MANUAL_TARGET_FILES})
        ADD_CUSTOM_COMMAND(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_FILE} "$<TARGET_FILE_DIR:TrenchBroom>/manual/"
        )
    ENDFOREACH(MANUAL_FILE)
ENDIF()

# Common CPack configuration
SET(APP_PACKAGE_FILE_NAME "TrenchBroom-${APP_PLATFORM_NAME}-${GIT_DESCRIBE}-${CMAKE_BUILD_TYPE}")
SET(APP_PACKAGE_DIR_NAME "$ENV{DROPBOX}/TrenchBroom/")
SET(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TrenchBroom Level Editor")
SET(CPACK_PACKAGE_VENDOR "Kristian Duske")
IF (CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(CPACK_STRIP_FILES YES)
ELSE()
    SET(CPACK_STRIP_FILES FALSE)
ENDIF()

# Platform specific CPack configuration
IF(WIN32)
    IF(COMPILER_IS_MSVC)
        # SET(CMAKE_INSTALLL_DEBUG_LIBRARIES OFF)
        # INCLUDE(InstallRequiredSystemLibraries)
    ENDIF()

    FILE(GLOB WIN_LIBS "${LIB_BIN_DIR}/win32/*.dll")
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        WX_LIB_TO_DLL(${WX_cored} _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_core)
        WX_LIB_TO_DLL(${WX_based} _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_base)
        WX_LIB_TO_DLL(${WX_advd}  _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_adv)
        WX_LIB_TO_DLL(${WX_gld}   _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_gl)
    ELSE()
        WX_LIB_TO_DLL(${WX_core} _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_core)
        WX_LIB_TO_DLL(${WX_base} _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_base)
        WX_LIB_TO_DLL(${WX_adv}  _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_adv)
        WX_LIB_TO_DLL(${WX_gl}   _${WX_LIB_DIR_PREFIX} WIN_LIB_WX_gl)
    ENDIF()
    
    # Copy PDB files (msvc debug symbols)
    IF(COMPILER_IS_MSVC)
        IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            # Get paths to wxwidgets debug symbols
            STRING(REGEX REPLACE "dll$" "pdb" WIN_PDB_WX_core ${WIN_LIB_WX_core})
            STRING(REGEX REPLACE "dll$" "pdb" WIN_PDB_WX_base ${WIN_LIB_WX_base})
            STRING(REGEX REPLACE "dll$" "pdb" WIN_PDB_WX_adv ${WIN_LIB_WX_adv})
            STRING(REGEX REPLACE "dll$" "pdb" WIN_PDB_WX_gl ${WIN_LIB_WX_gl})
        
            INSTALL(FILES
                "$<TARGET_FILE_DIR:TrenchBroom>/TrenchBroom.pdb"
                ${WIN_PDB_WX_core}
                ${WIN_PDB_WX_base}
                ${WIN_PDB_WX_adv}
                ${WIN_PDB_WX_gl}
                DESTINATION . COMPONENT TrenchBroom)
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
            INSTALL(FILES
                "$<TARGET_FILE_DIR:TrenchBroom>/TrenchBroom-stripped.pdb"
                DESTINATION . COMPONENT TrenchBroom)
        ENDIF()
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
        ${DOC_MANUAL_TARGET_FILES}
        DESTINATION manual COMPONENT TrenchBroom)
    INSTALL(DIRECTORY
        "${APP_DIR}/resources/graphics/images"
        "${APP_DIR}/resources/fonts"
        "${APP_DIR}/resources/games"
        "${APP_DIR}/resources/shader"
        DESTINATION . COMPONENT TrenchBroom)
    SET(CPACK_GENERATOR "7Z")
    SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)

    SET(CPACK_PACKAGE_FILE_EXT "7z")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateChecksum.bat.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum.bat" @ONLY)
ELSEIF(APPLE)
    INSTALL(TARGETS TrenchBroom BUNDLE DESTINATION . COMPONENT TrenchBroom)
    SET(CPACK_GENERATOR "DragNDrop")

    SET(CPACK_PACKAGE_FILE_EXT "dmg")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum.sh" @ONLY)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # add architecture to filename
    SET(APP_PACKAGE_FILE_NAME "${APP_PACKAGE_FILE_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
    SET(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})

    # generate deb and rpm packages
    SET(CPACK_GENERATOR "DEB;RPM")
    SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    SET(LINUX_RESOURCE_LOCATION "share/TrenchBroom")
    SET(LINUX_TARGET_RESOURCE_DIRECTORY ${CPACK_PACKAGING_INSTALL_PREFIX}/${LINUX_RESOURCE_LOCATION})

    # configure install scripts
    CONFIGURE_FILE(${APP_DIR}/resources/linux/postinst ${CMAKE_CURRENT_BINARY_DIR}/linux/postinst @ONLY)
    CONFIGURE_FILE(${APP_DIR}/resources/linux/prerm ${CMAKE_CURRENT_BINARY_DIR}/linux/prerm @ONLY)
    CONFIGURE_FILE(${APP_DIR}/resources/linux/postrm ${CMAKE_CURRENT_BINARY_DIR}/linux/postrm @ONLY)

    # add files
    INSTALL(TARGETS TrenchBroom RUNTIME DESTINATION bin COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fonts"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/games"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/manual"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/images"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(DIRECTORY "${APP_DIR}/resources/linux/icons"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom FILES_MATCHING PATTERN "*.png")
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/gpl.txt"                     DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(FILES "${APP_DIR}/resources/linux/copyright"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    INSTALL(FILES "${APP_DIR}/resources/linux/trenchbroom.desktop"  DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)

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
    SET(CPACK_RPM_SPEC_INSTALL_POST "/bin/true") # prevents stripping of debug symbols during rpmbuild

    SET(CPACK_PACKAGE_FILE_EXT "rpm")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum_rpm.sh" @ONLY)

    SET(CPACK_PACKAGE_FILE_EXT "deb")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum_deb.sh" @ONLY)
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/PrintDebianDependencies.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/print_debian_dependencies.sh" @ONLY)
ENDIF()
INCLUDE(CPack)
