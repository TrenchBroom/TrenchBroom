include("${UTILS}")

get_filename_component(SRC_DIR ${SRC} DIRECTORY)
get_git_describe("${GIT_EXECUTABLE}" "${SRC_DIR}" GIT_DESCRIBE)
get_app_version(GIT_DESCRIBE APP_VERSION_YEAR APP_VERSION_NUMBER APP_VERSION_RC)
get_app_version_str(APP_VERSION_YEAR APP_VERSION_NUMBER APP_VERSION_RC APP_VERSION_STR)
get_build_platform(APP_PLATFORM_NAME)

configure_file("${SRC}" "${DST}" @ONLY)
