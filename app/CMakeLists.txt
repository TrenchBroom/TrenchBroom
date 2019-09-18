include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateManual.cmake)

set(APP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(APP_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(APP_SOURCE "${APP_SOURCE_DIR}/Main.cpp")
set(APP_SOURCE ${APP_SOURCE} ${DOC_MANUAL_TARGET_FILES} ${DOC_MANUAL_IMAGES_TARGET_FILES})

# OS X app bundle configuration, must happen before the executable is added
if(APPLE)
    # Configure icons
    set(MACOSX_ICON_FILES "${APP_RESOURCE_DIR}/mac/icons/AppIcon.icns" "${APP_RESOURCE_DIR}/mac/icons/DocIcon.icns")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_ICON_FILES})
    set_source_files_properties(${MACOSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # Configure button bitmaps etc.
    file(GLOB_RECURSE MACOSX_IMAGE_FILES "${APP_RESOURCE_DIR}/graphics/images/*.png")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_IMAGE_FILES})
    set_source_files_properties(${MACOSX_IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/images)

    # Configure fonts
    file(GLOB_RECURSE MACOSX_FONT_FILES "${APP_RESOURCE_DIR}/fonts/*.*")
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_FONT_FILES})
    set_source_files_properties(${MACOSX_FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/fonts)

    # Configure game resources
    # Collect all game resources
    file(GLOB_RECURSE MACOSX_GAME_FILES RELATIVE ${APP_RESOURCE_DIR}/games "${APP_RESOURCE_DIR}/games/*")

    # Set correct locations for game files
    foreach(GAME_FILE ${MACOSX_GAME_FILES})
        get_filename_component(GAME_FILE_DIR "${GAME_FILE}" DIRECTORY)
        set(GAME_FILE "${APP_RESOURCE_DIR}/games/${GAME_FILE}")

        set(APP_SOURCE ${APP_SOURCE} ${GAME_FILE})
        set_source_files_properties(${GAME_FILE} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/${GAME_FILE_DIR})
    endforeach()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Collect all untested game resources
        file(GLOB_RECURSE MACOSX_GAME_TESTING_FILES RELATIVE ${APP_RESOURCE_DIR}/games-testing "${APP_RESOURCE_DIR}/games-testing/*")

        # Set correct locations for untested game files
        foreach(GAME_FILE ${MACOSX_GAME_TESTING_FILES})
            get_filename_component(GAME_FILE_DIR "${GAME_FILE}" DIRECTORY)
            set(GAME_FILE "${APP_RESOURCE_DIR}/games-testing/${GAME_FILE}")

            set(APP_SOURCE ${APP_SOURCE} ${GAME_FILE})
            set_source_files_properties(${GAME_FILE} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/games/${GAME_FILE_DIR})
        endforeach()
    endif()

    # Configure shaders
    # Collect all shaders
    file(GLOB_RECURSE MACOSX_SHADER_FILES
            "${APP_RESOURCE_DIR}/shader/*.fragsh"
            "${APP_RESOURCE_DIR}/shader/*.vertsh")
    set_source_files_properties(${MACOSX_SHADER_FILES} PROPERTIES  MACOSX_PACKAGE_LOCATION Resources/shader)
    set(APP_SOURCE ${APP_SOURCE} ${MACOSX_SHADER_FILES})

    # Configure manual files
    set_source_files_properties(${DOC_MANUAL_TARGET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual)
    set_source_files_properties(${DOC_MANUAL_IMAGES_TARGET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/manual/images)
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

# Create the executable target and link with libraries
add_executable(TrenchBroom WIN32 MACOSX_BUNDLE ${APP_SOURCE})
target_link_libraries(TrenchBroom common) # common already comes with all its dependencies, no need to repeat them here
set_target_properties(TrenchBroom PROPERTIES AUTOMOC TRUE)

# Add dependency to generate the manual during build
add_dependencies(TrenchBroom GenerateManual)

# Enable ASAN if possible and requested
if(COMPILER_IS_GNU AND TB_ENABLE_ASAN)
    target_link_libraries(TrenchBroom asan)
endif()

# Extract version information and configure CPack with it
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
get_build_platform(APP_PLATFORM_NAME)

if(APPLE)
    # Configure variables that are substituted into the plist
    # Set CFBundleExecutable
    set_target_properties(TrenchBroom PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}")
    # Set CFBundleName, which controls the application menu label
    set_target_properties(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "TrenchBroom")
    # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
    set_target_properties(TrenchBroom PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    # Set CFBundleVersion to the git describe output. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
    set_target_properties(TrenchBroom PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${GIT_DESCRIBE}")

    # Set the path to the plist template
    set_target_properties(TrenchBroom PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${APP_RESOURCE_DIR}/mac/TrenchBroom-Info.plist")

    # Configure the XCode generator project
    set_xcode_attributes(TrenchBroom)

    # If requested, run macdeployqt which copies the Qt frameworks into the TrenchBroom.app bundle
    # and modifies the TrenchBroom executable to refer to these private copies of the framework instead
    # of the ones in /usr/local. This is slow so only do it if requested.
    if(TB_RUN_MACDEPLOYQT)
        message(STATUS "macdeployqt requested")

        # Get macdeployqt path (hack)
        get_target_property(TB_QMAKE_PATH Qt5::qmake IMPORTED_LOCATION)
        string(REPLACE "qmake" "macdeployqt" TB_MACDEPLOYQT_PATH "${TB_QMAKE_PATH}")

        add_custom_command(TARGET TrenchBroom POST_BUILD
                COMMAND "${TB_MACDEPLOYQT_PATH}" "$<TARGET_FILE_DIR:TrenchBroom>/../..")
    endif()
endif()

# Copy some Windows-specific resources
if(WIN32)
    # Copy icons to target dir
    add_custom_command(TARGET TrenchBroom PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/AppIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/DocIcon.ico"    "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/WindowIcon.ico" "${CMAKE_CURRENT_BINARY_DIR}")

    # Copy DLLs to app directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage>" "$<TARGET_FILE_DIR:TrenchBroom>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "$<TARGET_FILE_DIR:TrenchBroom>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Widgets>" "$<TARGET_FILE_DIR:TrenchBroom>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Gui>" "$<TARGET_FILE_DIR:TrenchBroom>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Core>" "$<TARGET_FILE_DIR:TrenchBroom>")

    # Copy icons to resources directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/AppIcon.ico"    "$<TARGET_FILE_DIR:TrenchBroom>/Resources/AppIcon.ico"
            COMMAND ${CMAKE_COMMAND} -E copy "${APP_RESOURCE_DIR}/win32/icons/WindowIcon.ico" "$<TARGET_FILE_DIR:TrenchBroom>/Resources/WindowIcon.ico")
endif()

# Generate a small stripped PDB for release builds so we get stack traces with symbols
if(COMPILER_IS_MSVC)
    set_target_properties(TrenchBroom PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /PDBSTRIPPED:Release/TrenchBroom-stripped.pdb /PDBALTPATH:TrenchBroom-stripped.pdb")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
    # Make executable name conventional lowercase on linux
    set_target_properties(TrenchBroom PROPERTIES OUTPUT_NAME "trenchbroom")
endif()

# Set up the resources and DLLs for the executable
if(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
    # Copy button images to resources directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/graphics/images" "$<TARGET_FILE_DIR:TrenchBroom>/images")

    # Copy fonts to resources directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/fonts" "$<TARGET_FILE_DIR:TrenchBroom>/fonts")

    # Copy game files to resources directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games/" "$<TARGET_FILE_DIR:TrenchBroom>/games")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Copy untested game files to resources directory
        add_custom_command(TARGET TrenchBroom POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/games-testing/" "$<TARGET_FILE_DIR:TrenchBroom>/games")
    endif()

    # Copy shader files to resources directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${APP_RESOURCE_DIR}/shader" "$<TARGET_FILE_DIR:TrenchBroom>/shader")

    # Copy manual files to resource directory
    add_custom_command(TARGET TrenchBroom POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TrenchBroom>/manual/"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TrenchBroom>/manual/images")

    foreach(MANUAL_FILE ${DOC_MANUAL_TARGET_FILES})
        add_custom_command(TARGET TrenchBroom POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_FILE} "$<TARGET_FILE_DIR:TrenchBroom>/manual/")
    endforeach(MANUAL_FILE)

    foreach(MANUAL_IMAGE_FILE ${DOC_MANUAL_IMAGES_TARGET_FILES})
        add_custom_command(TARGET TrenchBroom POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${MANUAL_IMAGE_FILE} "$<TARGET_FILE_DIR:TrenchBroom>/manual/images/")
    endforeach(MANUAL_IMAGE_FILE)
endif()

# Common CPack configuration
set(APP_PACKAGE_FILE_NAME "TrenchBroom-${APP_PLATFORM_NAME}-${GIT_DESCRIBE}-${CMAKE_BUILD_TYPE}")
set(APP_PACKAGE_DIR_NAME "$ENV{DROPBOX}/TrenchBroom/")
set(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TrenchBroom Level Editor")
set(CPACK_PACKAGE_VENDOR "Kristian Duske")
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
    install(FILES
            $<TARGET_FILE:freeimage>
            $<TARGET_FILE:freetype>
            $<TARGET_FILE:Qt5::Widgets>
            $<TARGET_FILE:Qt5::Gui>
            $<TARGET_FILE:Qt5::Core>
            DESTINATION . COMPONENT TrenchBroom)

    # file(GLOB WIN_LIBS "${LIB_BIN_DIR}/win32/*.dll")

    #Qt plugins
    install(FILES
            $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
            DESTINATION platforms COMPONENT TrenchBroom)
    install(FILES
            $<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>
            DESTINATION styles COMPONENT TrenchBroom)

    # install(FILES
    #         ${WIN_LIBS}
    #         DESTINATION . COMPONENT TrenchBroom)
    install(FILES
            ${DOC_MANUAL_TARGET_FILES}
            DESTINATION manual COMPONENT TrenchBroom)
    install(FILES
            ${DOC_MANUAL_IMAGES_TARGET_FILES}
            DESTINATION manual/images COMPONENT TrenchBroom)
    install(DIRECTORY
            "${APP_DIR}/resources/graphics/images"
            "${APP_DIR}/resources/fonts"
            "${APP_DIR}/resources/games"
            "${APP_DIR}/resources/shader"
            DESTINATION . COMPONENT TrenchBroom)
    set(CPACK_GENERATOR "7Z")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)

    set(CPACK_PACKAGE_FILE_EXT "7z")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.bat.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum.bat" @ONLY)
elseif(APPLE)
    install(TARGETS TrenchBroom BUNDLE DESTINATION . COMPONENT TrenchBroom)
    set(CPACK_GENERATOR "DragNDrop")

    set(CPACK_PACKAGE_FILE_EXT "dmg")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateChecksum.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_checksum.sh" @ONLY)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # add architecture to filename
    set(APP_PACKAGE_FILE_NAME "${APP_PACKAGE_FILE_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_PACKAGE_FILE_NAME ${APP_PACKAGE_FILE_NAME})

    # generate deb and rpm packages
    set(CPACK_GENERATOR "DEB;RPM")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    set(LINUX_RESOURCE_LOCATION "share/TrenchBroom")
    set(LINUX_TARGET_RESOURCE_DIRECTORY ${CPACK_PACKAGING_INSTALL_PREFIX}/${LINUX_RESOURCE_LOCATION})

    # configure install scripts
    configure_file(${APP_DIR}/resources/linux/postinst ${CMAKE_CURRENT_BINARY_DIR}/linux/postinst @ONLY)
    configure_file(${APP_DIR}/resources/linux/prerm ${CMAKE_CURRENT_BINARY_DIR}/linux/prerm @ONLY)
    configure_file(${APP_DIR}/resources/linux/postrm ${CMAKE_CURRENT_BINARY_DIR}/linux/postrm @ONLY)

    # add files
    install(TARGETS TrenchBroom RUNTIME DESTINATION bin COMPONENT TrenchBroom)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fonts"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/games"           DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/manual"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/images"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader"          DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(DIRECTORY "${APP_DIR}/resources/linux/icons"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom FILES_MATCHING PATTERN "*.png")
    install(FILES "${CMAKE_SOURCE_DIR}/gpl.txt"                     DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(FILES "${APP_DIR}/resources/linux/copyright"            DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)
    install(FILES "${APP_DIR}/resources/linux/trenchbroom.desktop"  DESTINATION ${LINUX_RESOURCE_LOCATION} COMPONENT TrenchBroom)

    # deb package specifics
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
    # The following dependencies were generated by running with CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON, and then adding "libqt5widgets5 (>= 5.9.5)" as an alternate for qt59base.
    # Our CI builds on Ubuntu 16.04 need to pull Qt 5.9.5 from ppa:beineri/opt-qt595-xenial, which is where the "qt59base" package comes from.
    # We want to support Ubuntu 18.04, though, which should have libqt5widgets5 5.9.5.
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libfreeimage3, libfreetype6 (>= 2.2.1), libgcc1 (>= 1:3.0), libgl1-mesa-glx | libgl1, libstdc++6 (>= 5.2), qt59base | libqt5widgets5 (>= 5.9.5)")
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

include(CPack)
