FIND_PROGRAM(CPPCHECK_EXE cppcheck)

IF (CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
    MESSAGE(STATUS "Could not find cppcheck, skipping checks")
    ADD_CUSTOM_TARGET(
        cppcheck
        COMMENT "skipping cppcheck"
    )
ELSE()
    LIST(APPEND CPPCHECK_COMMON_ARGS
        --enable=warning,performance,portability
        --verbose
        --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck.suppr
        --std=c++11
        --language=c++
        -DMAIN=main
        -I ${VECMATH_INCLUDE_DIR}
    )

    LIST(APPEND CPPCHECK_ARGS
        ${CPPCHECK_COMMON_ARGS}
        --error-exitcode=1
        ${COMMON_SOURCE_DIR}
        2> ./cppcheck-errors.txt
    )

    MESSAGE(STATUS "Using cppcheck found at ${CPPCHECK_EXE}")
    STRING (REPLACE ";" " " CPPCHECK_ARGS_STR "${CPPCHECK_ARGS}")
    ADD_CUSTOM_TARGET(
        cppcheck
        COMMAND ${CPPCHECK_EXE} "--version"
        COMMAND ${CPPCHECK_EXE} ${CPPCHECK_ARGS}
        COMMENT "running ${CPPCHECK_EXE} ${CPPCHECK_ARGS_STR}"
    )

    FIND_PROGRAM(CPPCHECK_HTMLREPORT_EXE cppcheck-htmlreport)
    IF (NOT CPPCHECK_HTMLREPORT_EXE STREQUAL "CPPCHECK_HTMLREPORT_EXE-NOTFOUND")
        LIST(APPEND CPPCHECK_REP_ARGS
            ${CPPCHECK_COMMON_ARGS}
            --error-exitcode=0
            --xml
            ${COMMON_SOURCE_DIR}
            2> ./cppcheck-errors.xml
        )

        LIST(APPEND CPPCHECK_HTMLREPORT_ARGS
            --file cppcheck-errors.xml
            --report-dir=cppcheck_report
            --source-dir=${CMAKE_SOURCE_DIR}
        )

        MESSAGE(STATUS "Using cppcheck-htmlreport found at ${CPPCHECK_HTMLREPORT_EXE}")
        STRING (REPLACE ";" " " CPPCHECK_HTMLREPORT_ARGS_STR "${CPPCHECK_HTMLREPORT_ARGS}")
        ADD_CUSTOM_TARGET(
            cppcheck-report
            COMMAND ${CPPCHECK_EXE} ${CPPCHECK_REP_ARGS}
            COMMAND ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS}
            COMMENT "running ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS_STR}"
        )
    ENDIF()
ENDIF()
