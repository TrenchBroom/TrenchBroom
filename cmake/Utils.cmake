macro(FIX_WIN32_PATH VARNAME)
    if(WIN32)
        STRING(REPLACE "/" "\\" ${VARNAME} "${${VARNAME}}")
    endif()
endmacro(FIX_WIN32_PATH)

macro(ADD_TARGET_PROPERTY TARGET PROPERTY VALUE)
    get_target_property(CURRENT_VALUE ${TARGET} ${PROPERTY})
    if(NOT CURRENT_VALUE)
        set_property(TARGET ${TARGET} PROPERTY ${PROPERTY} ${VALUE})
    else()
        LIST(APPEND CURRENT_VALUE ${VALUE})
        set_property(TARGET ${TARGET} PROPERTY ${PROPERTY} ${CURRENT_VALUE})
    endif()
endmacro(ADD_TARGET_PROPERTY)

macro(GET_APP_VERSION_STR VERSION_YEAR VERSION_NUMBER VERSION_RC VERSION_STR)
  if(NOT "${${VERSION_RC}}" STREQUAL "")
    set(${VERSION_STR} "${${VERSION_YEAR}}.${${VERSION_NUMBER}}-RC${${VERSION_RC}}")
  else()
  set(${VERSION_STR} "${${VERSION_YEAR}}.${${VERSION_NUMBER}}")
  endif()
endmacro(GET_APP_VERSION_STR)

macro(GET_APP_VERSION GIT_DESCRIBE VERSION_YEAR VERSION_NUMBER VERSION_RC)
    STRING(REGEX MATCH "v([0-9][0-9][0-9][0-9])[.]([0-9]+)(-RC([0-9]+))?" GIT_DESCRIBE_MATCH "${${GIT_DESCRIBE}}")

    if(GIT_DESCRIBE_MATCH)
        set(${VERSION_YEAR} ${CMAKE_MATCH_1})
        set(${VERSION_NUMBER} ${CMAKE_MATCH_2})
        if(CMAKE_MATCH_COUNT GREATER 2)
          set(${VERSION_RC} ${CMAKE_MATCH_4})
        endif()
    else()
        # GIT_DESCRIBE is not a release tag following the "v0000.0" format.
        set(${VERSION_YEAR} "0")
        set(${VERSION_NUMBER} "0")
    endif()
endmacro(GET_APP_VERSION)

macro(GET_GIT_DESCRIBE GIT SOURCE_DIR GIT_DESCRIBE)
    # When building tags, GitHub Actions checks them out as lightweight tags even if the original tag was annotated.
    # Pass --tags to enable git describe to match non-annotated tags.
    execute_process(COMMAND ${GIT} describe --dirty --tags WORKING_DIRECTORY ${SOURCE_DIR} OUTPUT_VARIABLE ${GIT_DESCRIBE} OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(${GIT_DESCRIBE})
        message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from git describe")
    endif()

    # On GitHub Actions, "git describe" will fail due to it being a shallow clone (for PR and branch builds),
    # and it is also useful to record in the version description whether it's a branch or PR build.
    # In this case, use:
    #  - TB_PULL_REQUEST_HEAD_SHA (ci.yml sets to the last commit on the PR branch before the merge commit)
    #  - or GITHUB_SHA (set by GitHub Actions)
    # and GITHUB_REF to construct a descriptive version string.
    # See: https://docs.github.com/en/actions/reference/environment-variables#default-environment-variables
    if(NOT ${GIT_DESCRIBE})
        if((NOT ("$ENV{GITHUB_SHA}" STREQUAL "")) AND (NOT ("$ENV{TB_PULL_REQUEST_HEAD_SHA}" STREQUAL "")))
            set(${GIT_DESCRIBE} "unstable-$ENV{GITHUB_REF}-$ENV{TB_PULL_REQUEST_HEAD_SHA}")
            # Replace / with _ because we need GIT_DESCRIBE to be valid to put in a filename for the final package
            string(REPLACE "/" "_" ${GIT_DESCRIBE} ${${GIT_DESCRIBE}})
            message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from environment variables TB_PULL_REQUEST_HEAD_SHA and GITHUB_REF")
        endif()
    endif()

    if(NOT ${GIT_DESCRIBE})
        if((NOT ("$ENV{GITHUB_SHA}" STREQUAL "")) AND (NOT ("$ENV{GITHUB_REF}" STREQUAL "")))
            set(${GIT_DESCRIBE} "unstable-$ENV{GITHUB_REF}-$ENV{GITHUB_SHA}")
            string(REPLACE "/" "_" ${GIT_DESCRIBE} ${${GIT_DESCRIBE}})
            message(STATUS "Using version description \"${${GIT_DESCRIBE}}\" from environment variables GITHUB_SHA and GITHUB_REF")
        endif()
    endif()

    if(NOT ${GIT_DESCRIBE})
        set(${GIT_DESCRIBE} "unknown")
        message(WARNING "Unable to determine version description; using \"${${GIT_DESCRIBE}}\"")
    endif()
endmacro(GET_GIT_DESCRIBE)

macro(GET_BUILD_PLATFORM PLATFORM_NAME)
    if(WIN32)
        set(${PLATFORM_NAME} "Win64")
    elseif(APPLE)
        set(${PLATFORM_NAME} "macOS")
    elseif(UNIX)
        set(${PLATFORM_NAME} "Linux")
    else()
        set(${PLATFORM_NAME} "Unknown")
    endif()
endmacro(GET_BUILD_PLATFORM)
