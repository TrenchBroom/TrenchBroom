add_executable(DumpShortcuts
  ${CMAKE_CURRENT_SOURCE_DIR}/src/KeyStrings.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp

  ${INDEX_OUTPUT_PATH}
  ${DOC_MANUAL_TARGET_FILES_ABSOLUTE}
  ${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}
  ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE}
)

target_link_libraries(DumpShortcuts PRIVATE CompilerConfig)

target_link_libraries(DumpShortcuts PRIVATE 
  KdLib
  TbBaseLib
  TbMdlLib
  TbPreferencesLib
  TbUiLib
  Qt6::Widgets
)
set_target_properties(DumpShortcuts PROPERTIES AUTOMOC TRUE)

if(WIN32)
    # Copy DLLs to app directory
    add_custom_command(TARGET DumpShortcuts POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:assimp::assimp>" "$<TARGET_FILE_DIR:DumpShortcuts>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage::FreeImage>" "$<TARGET_FILE_DIR:DumpShortcuts>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "$<TARGET_FILE_DIR:DumpShortcuts>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:tinyxml2::tinyxml2>" "$<TARGET_FILE_DIR:DumpShortcuts>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:miniz::miniz>" "$<TARGET_FILE_DIR:DumpShortcuts>"
    )
    
    # Run windeployqt which copies the Qt dlls into the deployable folder, along with necessary runtime dlls by the compiler
    if(NOT TB_SKIP_WINDEPLOYQT)
        message(STATUS "windeployqt requested")

        # Get windeployqt path (hack)
        get_target_property(TB_QMAKE_PATH Qt6::qmake IMPORTED_LOCATION)
        string(REPLACE "qmake" "windeployqt" TB_WINDEPLOYQT_PATH "${TB_QMAKE_PATH}")
        message(STATUS "windeployqt found: ${TB_WINDEPLOYQT_PATH}")

        add_custom_command(TARGET DumpShortcuts POST_BUILD
                COMMAND "${TB_WINDEPLOYQT_PATH}"
                        --no-compiler-runtime
                        --no-translations
                        "$<TARGET_FILE_DIR:DumpShortcuts>")
    else()
        message(STATUS "windeployqt skipped")
    endif()        
endif()
