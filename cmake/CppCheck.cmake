LIST(APPEND CPPCHECK_ARGS
    --enable=warning,performance,portability
    --verbose
    --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck.suppr
    --std=c++11
    --language=c++
    --error-exitcode=1
    -DMAIN=main
    -I ${VECMATH_INCLUDE_DIR}
    ${COMMON_SOURCE_DIR}
)

FIND_FILE(CPPCHECK_EXE cppchecdk)

IF (CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
    MESSAGE(STATUS "Could not find cppcheck, skipping checks")
    ADD_CUSTOM_TARGET(
        cppcheck
        COMMENT "skipping cppcheck"
    )
ELSE()
    MESSAGE(STATUS "Using cppcheck found at ${CPPCHECK_EXE}")
    ADD_CUSTOM_TARGET(
        cppcheck
        COMMAND ${CPPCHECK_EXE} ${CPPCHECK_ARGS}
        COMMENT "running cppcheck"
    )
ENDIF()
