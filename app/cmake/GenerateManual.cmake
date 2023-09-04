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

# Generate manual
# 1. Create target directory
# 2. Run pandoc to create a temporary HTML file
# 3. Run AddVersionToManual.cmake on the temporary HTML file
# 4. Run TransformKeyboardShortcuts.cmake on the temporary HTML file
# 5. Copy the temporary HTML file to its target
# 6. Remove the temporary HTML file
add_custom_command(OUTPUT "${INDEX_OUTPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_TARGET_DIR}"
    COMMAND ${PANDOC_PATH} --standalone --toc --toc-depth=2 --template "${PANDOC_TEMPLATE_PATH}" --from=markdown --to=html5 -o "${PANDOC_OUTPUT_PATH}" "${PANDOC_INPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -DINPUT="${PANDOC_OUTPUT_PATH}" -DOUTPUT="${PANDOC_OUTPUT_PATH}" -P "${CMAKE_CURRENT_BINARY_DIR}/AddVersionToManual.cmake"
    COMMAND ${CMAKE_COMMAND} -DINPUT="${PANDOC_OUTPUT_PATH}" -DOUTPUT="${PANDOC_OUTPUT_PATH}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake"
    COMMAND ${CMAKE_COMMAND} -E copy "${PANDOC_OUTPUT_PATH}" "${INDEX_OUTPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -E remove "${PANDOC_OUTPUT_PATH}"
    DEPENDS "${PANDOC_TEMPLATE_PATH}" "${PANDOC_INPUT_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddVersionToManual.cmake.in"
)

# Dump the keyboard shortcuts
set(DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE "${DOC_MANUAL_TARGET_DIR}/shortcuts.js")
if(NOT XVFB_EXE)
    find_program(XVFB_EXE xvfb-run)
endif()
if(XVFB_EXE STREQUAL "XVFB_EXE-NOTFOUND")
    add_custom_command(
            OUTPUT "${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_TARGET_DIR}"
            COMMAND dump-shortcuts ARGS "${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}"
            DEPENDS dump-shortcuts
            VERBATIM)
else()
    add_custom_command(
            OUTPUT "${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_TARGET_DIR}"
            COMMAND "${XVFB_EXE}" ARGS "-a" "$<TARGET_FILE:dump-shortcuts>" "${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE}"
            DEPENDS dump-shortcuts
            VERBATIM)
endif()

# Collect resources and copy them to the correct locations
# DOC_MANUAL_SOURCE_FILES_ABSOLUTE contains the absolute paths to all source resource files
# DOC_MANUAL_SOURCE_IMAGE_FILES_RELATIVE contains the relative paths to all source resource files, relative to DOC_MANUAL_IMAGES_SOURCE_DIR
# DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE contains the absolute paths to all target resource files (used for dependency tracking)

# Collect resources
file(GLOB DOC_MANUAL_SOURCE_FILES_ABSOLUTE
    "${DOC_MANUAL_SOURCE_DIR}/*.css"
    "${DOC_MANUAL_SOURCE_DIR}/*.js"
)

# Get relative source and absolute target paths
foreach(MANUAL_SOURCE_FILE_ABSOLUTE ${DOC_MANUAL_SOURCE_FILES_ABSOLUTE})
    get_filename_component(MANUAL_SOURCE_FILE_NAME "${MANUAL_SOURCE_FILE_ABSOLUTE}" NAME)
    set(MANUAL_TARGET_FILE_ABSOLUTE "${DOC_MANUAL_TARGET_DIR}/${MANUAL_SOURCE_FILE_NAME}")

    file(RELATIVE_PATH MANUAL_SOURCE_FILE_RELATIVE "${DOC_MANUAL_SOURCE_DIR}" "${MANUAL_SOURCE_FILE_ABSOLUTE}")
    set(DOC_MANUAL_SOURCE_FILES_RELATIVE
        ${DOC_MANUAL_SOURCE_FILES_RELATIVE}
        "${MANUAL_SOURCE_FILE_RELATIVE}")

    set(DOC_MANUAL_TARGET_FILES_ABSOLUTE
        ${DOC_MANUAL_TARGET_FILES_ABSOLUTE}
        "${MANUAL_TARGET_FILE_ABSOLUTE}")
endforeach(MANUAL_SOURCE_FILE_ABSOLUTE)

# Copy the files using the relative paths (absolute paths would yield very long command lines which are then truncated by MSVC)
add_custom_command(OUTPUT ${DOC_MANUAL_TARGET_FILES_ABSOLUTE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_MANUAL_SOURCE_FILES_RELATIVE} "${DOC_MANUAL_TARGET_DIR}"
    DEPENDS ${DOC_MANUAL_SOURCE_FILES_ABSOLUTE}
    WORKING_DIRECTORY "${DOC_MANUAL_SOURCE_DIR}"
)

# Collect images and copy them to the correct locations
# DOC_MANUAL_SOURCE_IMAGE_FILES_ABSOLUTE contains the absolute paths to all source image files
# DOC_MANUAL_SOURCE_IMAGE_FILES_RELATIVE contains the relative paths to all source image files, relative to DOC_MANUAL_IMAGES_SOURCE_DIR
# DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE contains the absolute paths to all target image files (used for dependency tracking)

# Collect images
file(GLOB DOC_MANUAL_SOURCE_IMAGE_FILES_ABSOLUTE
    "${DOC_MANUAL_IMAGES_SOURCE_DIR}/*.png"
    "${DOC_MANUAL_IMAGES_SOURCE_DIR}/*.gif"
)

# Get relative source and absolute target paths
foreach(IMAGE_SOURCE_FILE_ABSOLUTE ${DOC_MANUAL_SOURCE_IMAGE_FILES_ABSOLUTE})
    get_filename_component(IMAGE_FILE_NAME "${IMAGE_SOURCE_FILE_ABSOLUTE}" NAME)
    set(IMAGE_TARGET_FILE "${DOC_MANUAL_IMAGES_TARGET_DIR}/${IMAGE_FILE_NAME}")

    file(RELATIVE_PATH IMAGE_SOURCE_FILE_RELATIVE "${DOC_MANUAL_IMAGES_SOURCE_DIR}" "${IMAGE_SOURCE_FILE_ABSOLUTE}")
    set(DOC_MANUAL_SOURCE_IMAGE_FILES_RELATIVE
        ${DOC_MANUAL_SOURCE_IMAGE_FILES_RELATIVE}
        "${IMAGE_SOURCE_FILE_RELATIVE}")

    set(DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE
        ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE}
        "${IMAGE_TARGET_FILE}"
    )
endforeach(IMAGE_SOURCE_FILE_ABSOLUTE)

# Copy the images using the relative paths (absolute paths would yield very long command lines which are then truncated by MSVC)
add_custom_command(OUTPUT ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_MANUAL_IMAGES_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_MANUAL_SOURCE_IMAGE_FILES_RELATIVE} "${DOC_MANUAL_IMAGES_TARGET_DIR}"
    DEPENDS ${DOC_MANUAL_SOURCE_IMAGE_FILES_ABSOLUTE}
    WORKING_DIRECTORY "${DOC_MANUAL_IMAGES_SOURCE_DIR}"
)

add_custom_target(GenerateManual DEPENDS ${INDEX_OUTPUT_PATH} ${DOC_MANUAL_TARGET_FILES_ABSOLUTE} ${DOC_MANUAL_SHORTCUTS_JS_TARGET_ABSOLUTE} ${DOC_MANUAL_TARGET_IMAGE_FILES_ABSOLUTE})
