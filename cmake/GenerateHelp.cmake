# Generate help documents
SET(DOC_DIR "${CMAKE_SOURCE_DIR}/app/resources/documentation")
SET(DOC_HELP_SOURCE_DIR "${DOC_DIR}/help")
SET(DOC_HELP_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/help")
ADD_CUSTOM_TARGET(GenerateHelp
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_HELP_TARGET_DIR}"
    COMMAND pandoc -s --toc --toc-depth=2 --template ${DOC_HELP_SOURCE_DIR}/template.html -o ${DOC_HELP_TARGET_DIR}/index.html ${DOC_HELP_SOURCE_DIR}/index.md
    COMMAND ${CMAKE_COMMAND} -DINPUT="${DOC_HELP_TARGET_DIR}/index.html" -DOUTPUT="${DOC_HELP_TARGET_DIR}/index.html" -P "${CMAKE_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake"
    COMMAND ${CMAKE_COMMAND} -E copy "${DOC_HELP_SOURCE_DIR}/default.css" "${DOC_HELP_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${DOC_HELP_SOURCE_DIR}/shortcuts.js" "${DOC_HELP_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${DOC_HELP_SOURCE_DIR}/shortcuts_helper.js" "${DOC_HELP_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${DOC_HELP_SOURCE_DIR}/images/" "${DOC_HELP_TARGET_DIR}/images"
)
