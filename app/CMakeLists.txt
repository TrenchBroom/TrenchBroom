include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateManual.cmake)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    option(APPIMAGE_BUILD "Prepare AppImage build" OFF)
endif()

set(APP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(APP_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(APP_SOURCE
    "${APP_SOURCE_DIR}/Main.cpp"
    "${APP_RESOURCE_DIR}/qrc/resources.qrc")

# OS X app bundle configuration, must happen before the executable is added
if(APPLE)
    # Configure icons
    set(MACOSX_ICON_FILES "${APP_RESOURCE_DIR}/mac/icons/AppIcon.icns" "${APP_RESOURCE_DIR}/mac/icons/DocIcon.icns")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_ICON_FILES})
    set_source_files_properties(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # Configure button bitmaps etc.
    file(GLOB_RECURSE MACOSX_IMAGE_FILES "${APP_RESOURCE_DIR}/graphics/images/*")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_IMAGE_FILES})
    set_source_files_properties(${MACOSX_IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/images)

    # Configure fonts
    file(GLOB_RECURSE MACOSX_FONT_FILES "${APP_RESOURCE_DIR}/fonts/*.*")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_FONT_FILES})
    set_source_files_properties(${MACOSX_FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/fonts)

    # Configure default resources
    # Collect all default resources
    file(GLOB_RECURSE MACOSX_DEFAULT_FILES RELATIVE ${APP_RESOURCE_DIR}/defaults "${APP_RESOURCE_DIR}/defaults/*")

    # Set correct locations for default resources
    foreach(DEFAULT_FILE ${MACOSX_DEFAULT_FILES})
        get_filename_component(DEFAULT_FILE_DIR "${DEFAULT_FILE}" DIRECTORY)
        set(DEFAULT_FILE "${APP_RESOURCE_DIR}/defaults/${DEFAULT_FILE}")

        set(APP_SOURCE ${APP_SOURCE} ${DEFAULT_FILE})
        set_source_files_properties(${DEFAULT_FILE} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/defaults/${DEFAULT_FILE_DIR})
    endforeach()

    # Configure game resources
    # Collect all game resources
    file(GLOB_RECURSE MACOSX_GAME_FILES RELATIVE ${APP_RESOURCE_DIR}/games "${APP_RESOURCE_DIR}/games/*")

    # Set correct locations for game files
    foreach(GAME_FILE ${MACOSX_GAME_FILES})
        get_filename_component(GAME_FILE_DIR "${GAME_FILE}" DIRECTORY)
        set(GAME_FILE "${APP_RESOURCE_DIR}/games/${GAME_FILE}")

        set(APP_SOURCE ${APP_SOURCE} ${GAME_FILE})
        set_source_files_properties(${GAME_FILE} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/${GAME_FILE_DIR} HEADER_FILE_ONLY ON)
    endforeach()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Collect all untested game resources
        file(GLOB_RECURSE MACOSX_GAME_TESTING_FILES RELATIVE ${APP_RESOURCE_DIR}/games-testing "${APP_RESOURCE_DIR}/games-testing/*")

        # Set correct locations for untested game files
        foreach(GAME_FILE ${MACOSX_GAME_TESTING_FILES})
            get_filename_component(GAME_FILE_DIR "${GAME_FILE}" DIRECTORY)
            set(GAME_FILE "${APP_RESOURCE_DIR}/games-testing/${GAME_FILE}")

            set(APP_SOURCE ${APP_SOURCE} ${GAME_FILE})
            set_source_files_properties(${GAME_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/games/${GAME_FILE_DIR} HEADER_FILE_ONLY ON)
        endforeach()
    endif()

    # Configure shaders
    # Collect all shaders
    file(GLOB_RECURSE MACOSX_SHADER_FILES
            "${APP_RESOURCE_DIR}/shader/*.fragsh"
            "${APP_RESOURCE_DIR}/shader/*.vertsh")
    set_source_files_properties(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_SHADER_FILES})

    # Configure Qt stylesheets
    file (GLOB_RECURSE MACOSX_STYLESHEET_FILES
            "${APP_RESOURCE_DIR}/stylesheets/*.qss")
    set_source_files_properties(${MACOSX_STYLESHEET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/stylesheets)
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_STYLESHEET_FILES})

    # Configure manual files
    set_source_files_properties(${INDEX_OUTPUT_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual)
    set_source_files_properties(${DOC_MANUAL_TARGET_FILES_ABSOLUTE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual)
    set_source_files_properties(${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual)
    set_source_files_properties(${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual/images)
endif()

# Set up resource compilation for Windows
if(WIN32)
    # Create the windows resource file
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/TrenchBroom.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/TrenchBroom.rc" @ONLY)

    if(COMPILER_IS_MSVC)
        set(APP_SOURCE ${APP_SOURCE} "${CMAKE_CURRENT_BINARY_DIR}/TrenchBroom.rc")
    elseif(MINGW)
        set(CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
        set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -i <SOURCE> -o <OBJECT>")
    endif()
endif()

# Set CPACK_PACKAGE_ which is used by CPack.
# Strip the "v" prefix off GIT_DESCRIBE (e.g. "v2020.1-RC1") to produce a CPACK_PACKAGE_VERSION of "2020.1-RC1"
# Debian wants the first character to be a number
string(REGEX REPLACE "^v(.*)" "\\1" CPACK_PACKAGE_VERSION "${GIT_DESCRIBE}")
get_build_platform(APP_PLATFORM_NAME)

macro(configure_app_target APP_TARGET_NAME SHOULD_BUILD_MANUAL)
    target_link_libraries(${APP_TARGET_NAME} common) # common already comes with all its dependencies, no need to repeat them here
    # Xcode "new build system" does't allow custom commands to be depended on by two unrelated targets
    # In this case, ${INDEX_OUTPUT_PATH} is depended on by TrenchBroom and GenerateManual
    # so we must make TrenchBroom depend on GenerateManual (see #3608)
    if (${SHOULD_BUILD_MANUAL})
        add_dependencies(${APP_TARGET_NAME} GenerateManual)
    endif()
    set_compiler_config(${APP_TARGET_NAME})

    set_target_properties(${APP_TARGET_NAME} PROPERTIES AUTORCC TRUE)

    if(APPLE)
        # Configure variables that are substituted into the plist
        # Set CFBundleExecutable
        set_target_properties(${APP_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}")
        # Set CFBundleName, which controls the application menu label
        set_target_properties(${APP_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "TrenchBroom")
        # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
        set_target_properties(${APP_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
        # Set CFBundleVersion to the git describe output. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
        set_target_properties(${APP_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${GIT_DESCRIBE}")

        # Set the path to the plist template
        set_target_properties(${APP_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${APP_RESOURCE_DIR}/mac/TrenchBroom-Info.plist")

        # If requested, run macdeployqt which copies the Qt frameworks into the TrenchBroom.app bundle
        # and modifies the TrenchBroom executable to refer to these private copies of the framework instead
        # of the ones in /usr/local. This is slow so only do it if requested.
        if(TB_RUN_MACDEPLOYQT)
            message(STATUS "macdeployqt requested")

            # Get macdeployqt path (hack)
            get_target_property(TB_QMAKE_PATH Qt5::qmake IMPORTED_LOCATION)
            string(REPLACE "qmake" "macdeployqt" TB_MACDEPLOYQT_PATH "${TB_QMAKE_PATH}")

            add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                    COMMAND "${TB_MACDEPLOYQT_PATH}" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>/../..")
        endif()
    endif()

    # Copy some Windows-specific resources
    if(WIN32)
        # Copy icons to target dir
        add_custom_command(TARGET ${APP_TARGET_NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/AppIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/DocIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/WindowIcon.ico" "${CMAKE_CURRENT_BINARY_DIR}")

        # Copy DLLs to app directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:assimp::assimp>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage::FreeImage>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:tinyxml2::tinyxml2>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:miniz::miniz>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:fmt::fmt>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GLEW::GLEW>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Widgets>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Gui>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Core>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Svg>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>/platforms"
                COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>/styles"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>/platforms"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>/styles")
    endif()

    # Generate a small stripped PDB for release builds so we get stack traces with symbols
    if(COMPILER_IS_MSVC)
        set_target_properties(${APP_TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/${APP_TARGET_NAME}-stripped.pdb /PDBALTPATH:${APP_TARGET_NAME}-stripped.pdb")
    endif()

    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
        # Make executable name conventional lowercase on linux
        string(TOLOWER ${APP_TARGET_NAME} APP_TARGET_NAME_LOWER)
        set_target_properties(${APP_TARGET_NAME} PROPERTIES OUTPUT_NAME ${APP_TARGET_NAME_LOWER})
    endif()

    # Set up the resources and DLLs for the executable
    set(APP_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:${APP_TARGET_NAME}>")

    if(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
        # Copy button images to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/graphics/images" "${APP_RESOURCE_DEST_DIR}/images")

        # Copy fonts to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/fonts" "${APP_RESOURCE_DEST_DIR}/fonts")

        # Copy default resources to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/defaults/" "${APP_RESOURCE_DEST_DIR}/defaults")

        # Copy game files to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games/" "${APP_RESOURCE_DEST_DIR}/games")

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            # Copy untested game files to resources directory
            add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games-testing/" "${APP_RESOURCE_DEST_DIR}/games")
        endif()

        # Copy shader files to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/shader" "${APP_RESOURCE_DEST_DIR}/shader")

        # Copy Qt stylesheet files to resources directory
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_RESOURCE_DEST_DIR}/stylesheets/"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/stylesheets" "${APP_RESOURCE_DEST_DIR}/stylesheets")

        if (${SHOULD_BUILD_MANUAL})
            # Copy manual files to resource directory
            add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_RESOURCE_DEST_DIR}/manual/"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_RESOURCE_DEST_DIR}/manual/images"
                    COMMAND ${CMAKE_COMMAND} -E copy "${INDEX_OUTPUT_PATH}" "${APP_RESOURCE_DEST_DIR}/manual/"
                    COMMAND ${CMAKE_COMMAND} -E copy "${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}" "${APP_RESOURCE_DEST_DIR}/manual/")

            foreach(MANUAL_FILE ${DOC_MANUAL_TARGET_FILES_ABSOLUTE})
                add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_FILE} "${APP_RESOURCE_DEST_DIR}/manual/")
            endforeach(MANUAL_FILE)

            foreach(MANUAL_IMAGE_FILE ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE})
                add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_IMAGE_FILE} "${APP_RESOURCE_DEST_DIR}/manual/images/")
            endforeach(MANUAL_IMAGE_FILE)
        endif()
    endif()
endmacro()

# Create the executable target and link with libraries
add_executable(TrenchBroom WIN32 MACOSX_BUNDLE)
target_sources(TrenchBroom PRIVATE ${APP_SOURCE} ${INDEX_OUTPUT_PATH} ${DOC_MANUAL_TARGET_FILES_ABSOLUTE} ${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE} ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE})
configure_app_target(TrenchBroom YES)

add_executable(TrenchBroom-nomanual WIN32 MACOSX_BUNDLE EXCLUDE_FROM_ALL)
target_sources(TrenchBroom-nomanual PRIVATE ${APP_SOURCE})
configure_app_target(TrenchBroom-nomanual NO)

# Common CPack configuration
set(APP_PACKAGE_FILE_NAME "TrenchBroom-${APP_PLATFORM_NAME}-${GIT_DESCRIBE}-${CMAKE_BUILD_TYPE}")
set(APP_PACKAGE_DIR_NAME "$ENV{DROPBOX}/TrenchBroom/")
set(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TrenchBroom Level Editor")
set(CPACK_PACKAGE_VENDOR "TrenchBroom")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # We want stack trace symbols even in our release builds, so don't strip
    set(CPACK_STRIP_FILES FALSE)
else()
    set(CPACK_STRIP_FILES FALSE)
endif()

# Platform specific CPack configuration
if(WIN32)
    if(COMPILER_IS_MSVC)
        # set(CMAKE_INSTALLL_DEBUG_LIBRARIES OFF)
        # INCLUDE(InstallRequiredSystemLibraries)
    endif()

    # Copy PDB files (msvc debug symbols)
    if(COMPILER_IS_MSVC)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            install(FILES
                    "$<TARGET_FILE_DIR:TrenchBroom>/TrenchBroom.pdb"
                    DESTINATION . COMPONENT TrenchBroom)
        elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
            install(FILES
                    "$<TARGET_FILE_DIR:TrenchBroom>/TrenchBroom-stripped.pdb"
                    DESTINATION . COMPONENT TrenchBroom)
        endif()
    endif()

    install(TARGETS TrenchBroom RUNTIME DESTINATION . COMPONENT TrenchBroom)
    install(CODE [[
        # The regexes are a workaround for https://gitlab.kitware.com/cmake/cmake/-/issues/22431
        file(GET_RUNTIME_DEPENDENCIES
            RESOLVED_DEPENDENCIES_VAR RESOLVED_DLLS
            PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*" "hvsifiletrust.dll"
            POST_EXCLUDE_REGEXES ".*[Ss][Yy][Ss][Tt][Ee][Mm]32.*"
            EXECUTABLES "$<TARGET_FILE:TrenchBroom>")

        message("Packaging .DLL's:")
        foreach(DLL ${RESOLVED_DLLS})
            message("${DLL}")
            file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" FILES "${DLL}")
        endforeach()
    ]])

    #Qt plugins
    install(FILES
            $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
            DESTINATION platforms COMPONENT TrenchBroom)
    install(FILES
            $<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>
            DESTINATION styles COMPONENT TrenchBroom)

    #Manual
    install(FILES ${INDEX_OUTPUT_PATH} DESTINATION manual COMPONENT TrenchBroom)
    install(FILES ${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE} DESTINATION manual COMPONENT TrenchBroom)
    install(FILES
            ${DOC_MANUAL_TARGET_FILES_ABSOLUTE}
            DESTINATION manual COMPONENT TrenchBroom)
    install(FILES
            ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE}
            DESTINATION manual/images COMPONENT TrenchBroom)
    install(DIRECTORY
            "${APP_DIR}/resources/graphics/images"
            "${APP_DIR}/resources/fonts"
            "${APP_DIR}/resources/defaults"
            "${APP_DIR}/resources/games"
            "${APP_DIR}/resources/shader"
            "${APP_DIR}/resources/stylesheets"
            DESTINATION . COMPONENT TrenchBroom)
    set(CPACK_GENERATOR "7Z")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)

    set(CPACK_PACKAGE_FILE_EXT "7z")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.bat.in" "${CMAKE_BINARY_DIR}/generate_checksum.bat" @ONLY)
elseif(APPLE)
    install(TARGETS TrenchBroom BUNDLE DESTINATION . COMPONENT TrenchBroom)
    set(CPACK_GENERATOR "DragNDrop")

    set(CPACK_PACKAGE_FILE_EXT "dmg")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum.sh" @ONLY)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # either build AppImage or generate deb and rpm packages
    if(APPIMAGE_BUILD)
        install(TARGETS TrenchBroom RUNTIME DESTINATION bin COMPONENT TrenchBroom)
        install(FILES "${APP_DIR}/resources/linux/trenchbroom.desktop" DESTINATION "share/applications" COMPONENT TrenchBroom)

        # filter and install all icons
        file(GLOB ICON_FILES "${APP_DIR}/resources/linux/icons/*.png")
        list(FILTER ICON_FILES EXCLUDE REGEX ".*tb_icon_orig\\.png$")
        foreach(icon ${ICON_FILES})
            if (${icon} MATCHES "_([0-9]+)\\.png$")
                set(IR ${CMAKE_MATCH_1})
                install(FILES "${APP_DIR}/resources/linux/icons/icon_${IR}.png" DESTINATION "share/icons/hicolor/${IR}x${IR}/apps/" COMPONENT TrenchBroom RENAME "trenchbroom.png")
            endif()
        endforeach()
    else()
        # add architecture to filename
        set(APP_PACKAGE_FILE_NAME "${APP_PACKAGE_FILE_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
        set(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})

        # generate deb and rpm packages
        set(CPACK_GENERATOR "DEB;RPM")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

        set(LINUX_RESOURCE_LOCATION "share/TrenchBroom")
        set(LINUX_TARGET_RESOURCE_DIRECTORY ${CPACK_PACKAGING_INSTALL_PREFIX}/${LINUX_RESOURCE_LOCATION})

        # add mime type definitions
        install(FILES "${APP_DIR}/resources/linux/trenchbroom.xml" DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/share/mime/packages/" COMPONENT TrenchBroom)

        # configure install scripts
        configure_file(${APP_DIR}/resources/linux/postinst ${CMAKE_CURRENT_BINARY_DIR}/linux/postinst @ONLY)
        configure_file(${APP_DIR}/resources/linux/prerm ${CMAKE_CURRENT_BINARY_DIR}/linux/prerm @ONLY)
        configure_file(${APP_DIR}/resources/linux/postrm ${CMAKE_CURRENT_BINARY_DIR}/linux/postrm @ONLY)

        # add files
        install(TARGETS TrenchBroom RUNTIME DESTINATION bin COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fonts"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/defaults"        DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/games"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/manual"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/images"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/stylesheets"     DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(DIRECTORY "${APP_DIR}/resources/linux/icons"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom FILES_MATCHING PATTERN "*.png")
        install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.txt"                 DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(FILES "${APP_DIR}/resources/linux/copyright"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
        install(FILES "${APP_DIR}/resources/linux/trenchbroom.desktop"  DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)

        # deb package specifics
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        # The following dependencies were generated by running with CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON and dumping the list of dependencies with dpkg-deb --info <debfile>,
        # and making the following modification(s)
        # - remove libgcc-s1 (>= 3.0)
        # set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27), libgl1, libqt5core5a (>= 5.9.0~beta), libqt5gui5 (>= 5.6.0~beta), libqt5svg5 (>= 5.6.0~beta), libqt5widgets5 (>= 5.7.0), libstdc++6 (>= 7)")
        set(CPACK_DEBIAN_PACKAGE_SECTION "games")
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kristianduske.com/trenchbroom/")
        set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/linux/postinst;${CMAKE_CURRENT_BINARY_DIR}/linux/prerm;${CMAKE_CURRENT_BINARY_DIR}/linux/postrm")

        # rpm package specifics
        set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
        set(CPACK_RPM_PACKAGE_GROUP "Applications/Editors")
        set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
        set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
        set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/postinst")
        set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/prerm")
        set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/linux/postrm")
        set(CPACK_RPM_SPEC_INSTALL_POST "/bin/true") # prevents stripping of debug symbols during rpmbuild

        set(CPACK_PACKAGE_FILE_EXT "rpm")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum_rpm.sh" @ONLY)

        set(CPACK_PACKAGE_FILE_EXT "deb")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum_deb.sh" @ONLY)

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrintLinuxPackageInfo.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/print_linux_package_info.sh" @ONLY)
    endif()

endif()


include(CPack)
