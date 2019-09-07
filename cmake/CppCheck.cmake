FIND_PROGRAM(CPPCHECK_EXE cppcheck)

IF (CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
    MESSAGE(STATUS "Could not find cppcheck, skipping checks")
    ADD_CUSTOM_COMMAND(
        cppcheck
        COMMENT "skipping cppcheck"
    )
ELSE()
    LIST(APPEND CPPCHECK_COMMON_ARGS
        --enable=warning,performance,portability
        --inline-suppr
        --std=c++17
        --language=c++
        -DMAIN=main
        -I ${VECMATH_INCLUDE_DIR}
    )

    LIST(APPEND CPPCHECK_ARGS
        ${CPPCHECK_COMMON_ARGS}
        --verbose
        --error-exitcode=1
        ${COMMON_SOURCE_DIR}
        2> ./cppcheck-errors.txt
    )

    MESSAGE(STATUS "Found cppcheck: ${CPPCHECK_EXE}")
    STRING (REPLACE ";" " " CPPCHECK_ARGS_STR "${CPPCHECK_ARGS}")
    ADD_CUSTOM_TARGET(
        cppcheck
        COMMAND ${CPPCHECK_EXE} "--version"
        COMMAND ${CPPCHECK_EXE} ${CPPCHECK_ARGS}
        COMMENT "running ${CPPCHECK_EXE} ${CPPCHECK_ARGS_STR}"
    )

    FIND_PROGRAM(CPPCHECK_HTMLREPORT_EXE cppcheck-htmlreport)
    IF (NOT CPPCHECK_HTMLREPORT_EXE STREQUAL "CPPCHECK_HTMLREPORT_EXE-NOTFOUND")
        LIST(APPEND CPPCHECK_XML_ARGS
            ${CPPCHECK_COMMON_ARGS}
            --error-exitcode=0
            --xml
            ${COMMON_SOURCE_DIR}
            2> ./cppcheck-errors.xml
        )

        STRING (REPLACE ";" " " CPPCHECK_XML_ARGS_STR "${CPPCHECK_XML_ARGS}")
        ADD_CUSTOM_COMMAND(
            OUTPUT cppcheck-errors.xml
            COMMAND ${CPPCHECK_EXE} "--version"
            COMMAND ${CPPCHECK_EXE} ${CPPCHECK_XML_ARGS}
            COMMENT "running ${CPPCHECK_EXE} ${CPPCHECK_XML_ARGS_STR}"
        )

        LIST(APPEND CPPCHECK_HTMLREPORT_ARGS
            --file cppcheck-errors.xml
            --report-dir=cppcheck_report
            --source-dir=${CMAKE_SOURCE_DIR}
        )

        MESSAGE(STATUS "Found cppcheck-htmlreport: ${CPPCHECK_HTMLREPORT_EXE}")
        STRING (REPLACE ";" " " CPPCHECK_HTMLREPORT_ARGS_STR "${CPPCHECK_HTMLREPORT_ARGS}")
        ADD_CUSTOM_TARGET(
            cppcheck-report
            COMMAND ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS}
            DEPENDS cppcheck-errors.xml
            COMMENT "running ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS_STR}"
        )
    ENDIF()
ENDIF()
