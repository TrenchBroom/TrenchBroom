# Generate help documents

# Create the cmake script for adding the version information to the manual
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddVersionToManual.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/AddVersionToManual.cmake" @ONLY)

# Configure paths
set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/documentation")
set(DOC_MANUAL_SOURCE_DIR "${DOC_DIR}/manual")
set(DOC_MANUAL_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen-manual")

set(DOC_MANUAL_IMAGES_SOURCE_DIR "${DOC_MANUAL_SOURCE_DIR}/images")
set(DOC_MANUAL_IMAGES_TARGET_DIR "${DOC_MANUAL_TARGET_DIR}/images")

set(PANDOC_TEMPLATE_PATH "${DOC_MANUAL_SOURCE_DIR}/template.html")
set(PANDOC_INPUT_PATH    "${DOC_MANUAL_SOURCE_DIR}/index.md")
set(PANDOC_OUTPUT_PATH   "${DOC_MANUAL_TARGET_DIR}/index.html.tmp")
set(INDEX_OUTPUT_PATH    "${DOC_MANUAL_TARGET_DIR}/index.html")

fix_win32_path(PANDOC_TEMPLATE_PATH)
fix_win32_path(PANDOC_INPUT_PATH)
fix_win32_path(PANDOC_OUTPUT_PATH)

# Create directories
add_custom_command(OUTPUT "${DOC_MANUAL_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_TARGET_DIR}"
    COMMENT "Creating directory ${DOC_MANUAL_TARGET_DIR}"
)

add_custom_command(OUTPUT "${DOC_MANUAL_IMAGES_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_IMAGES_TARGET_DIR}"
    COMMENT "Creating directory ${DOC_MANUAL_IMAGES_TARGET_DIR}"
)

# Generate manual
# 1. Run pandoc to create a temporary HTML file
# 2. Run AddVersionToManual.cmake on the temporary HTML file
# 3. Run TransformKeyboardShortcuts.cmake on the temporary HTML file
# 4. Copy the temporary HTML file to its target
# 4. Remove the temporary HTML file
add_custom_command(OUTPUT "${INDEX_OUTPUT_PATH}"
    COMMAND pandoc --standalone --toc --toc-depth=2 --template "${PANDOC_TEMPLATE_PATH}" --from=markdown --to=html5 -o "${PANDOC_OUTPUT_PATH}" "${PANDOC_INPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -DINPUT="${PANDOC_OUTPUT_PATH}" -DOUTPUT="${PANDOC_OUTPUT_PATH}" -P "${CMAKE_CURRENT_BINARY_DIR}/AddVersionToManual.cmake"
    COMMAND ${CMAKE_COMMAND} -DINPUT="${PANDOC_OUTPUT_PATH}" -DOUTPUT="${PANDOC_OUTPUT_PATH}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake"
    COMMAND ${CMAKE_COMMAND} -E copy "${PANDOC_OUTPUT_PATH}" "${INDEX_OUTPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -E remove "${PANDOC_OUTPUT_PATH}"
    DEPENDS "${DOC_MANUAL_TARGET_DIR}" "${PANDOC_TEMPLATE_PATH}" "${PANDOC_INPUT_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddVersionToManual.cmake.in"
    COMMENT "Creating manual file ${INDEX_OUTPUT_PATH}"
)

# Collect resources and copy them to the correct locations
file(GLOB DOC_MANUAL_SOURCE_FILES
    "${DOC_MANUAL_SOURCE_DIR}/*.css"
    "${DOC_MANUAL_SOURCE_DIR}/*.js"
)

foreach(MANUAL_SOURCE_FILE ${DOC_MANUAL_SOURCE_FILES})
    get_filename_component(MANUAL_SOURCE_FILE_NAME "${MANUAL_SOURCE_FILE}" NAME)
    set(MANUAL_TARGET_FILE "${DOC_MANUAL_TARGET_DIR}/${MANUAL_SOURCE_FILE_NAME}")

    set(DOC_MANUAL_TARGET_FILES
        ${DOC_MANUAL_TARGET_FILES}
        "${MANUAL_TARGET_FILE}"
    )
endforeach(MANUAL_SOURCE_FILE)

add_custom_command(OUTPUT ${DOC_MANUAL_TARGET_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_MANUAL_SOURCE_FILES} "${DOC_MANUAL_TARGET_DIR}"
    DEPENDS "${DOC_MANUAL_TARGET_DIR}" ${DOC_MANUAL_SOURCE_FILES}
    COMMENT "Copying manual files to ${DOC_MANUAL_TARGET_DIR}"
)

# Collect images and copy them to the correct locations
file(GLOB DOC_MANUAL_SOURCE_IMAGE_FILES
    "${DOC_MANUAL_IMAGES_SOURCE_DIR}/*.png"
    "${DOC_MANUAL_IMAGES_SOURCE_DIR}/*.gif"
)

foreach(IMAGE_SOURCE_FILE ${DOC_MANUAL_SOURCE_IMAGE_FILES})
    get_filename_component(IMAGE_FILE_NAME "${IMAGE_SOURCE_FILE}" NAME)
    set(IMAGE_TARGET_FILE "${DOC_MANUAL_IMAGES_TARGET_DIR}/${IMAGE_FILE_NAME}")

    set(DOC_MANUAL_TARGET_IMAGE_FILES
        ${DOC_MANUAL_TARGET_IMAGE_FILES}
        "${IMAGE_TARGET_FILE}"
    )
endforeach(IMAGE_SOURCE_FILE)

add_custom_command(OUTPUT ${DOC_MANUAL_TARGET_IMAGE_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_MANUAL_SOURCE_IMAGE_FILES} "${DOC_MANUAL_IMAGES_TARGET_DIR}"
    DEPENDS "${DOC_MANUAL_IMAGES_TARGET_DIR}" ${DOC_MANUAL_SOURCE_IMAGE_FILES}
    COMMENT "Copying image files to ${DOC_MANUAL_IMAGES_TARGET_DIR}"
)

add_custom_target(GenerateManual DEPENDS ${INDEX_OUTPUT_PATH} ${DOC_MANUAL_TARGET_FILES} ${DOC_MANUAL_TARGET_IMAGE_FILES})
