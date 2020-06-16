include(ProcessorCount)

if(NOT CPPCHECK_EXE)
    find_program(CPPCHECK_EXE cppcheck)
    if(CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
        message(STATUS "Could not find cppcheck, skipping checks")
    else()
        message(STATUS "Found cppcheck: ${CPPCHECK_EXE}")

        find_program(CPPCHECK_HTMLREPORT_EXE cppcheck-htmlreport)
        if(CPPCHECK_HTMLREPORT_EXE STREQUAL "CPPCHECK_HTMLREPORT_EXE-NOTFOUND")
            message(STATUS "Could not find cppcheck-htmlreport")
        else()
            message(STATUS "Found cppcheck-htmlreport: ${CPPCHECK_HTMLREPORT_EXE}")
        endif()
    endif()
endif()

if(CPPCHECK_EXE-NOTFOUND)
    message(STATUS "Could not find cppcheck, skipping checks")
    add_custom_target(
            cppcheck
            COMMENT "skipping cppcheck"
    )
else()
    ProcessorCount(CPU_COUNT)

    list(APPEND CPPCHECK_COMMON_ARGS
        -j${CPU_COUNT}
        --enable=warning,performance,portability
        --inline-suppr
        --std=c++17
        --language=c++
        -DMAIN=main
        -I${COMMON_SOURCE_DIR}
    )

    list(APPEND CPPCHECK_ARGS
        ${CPPCHECK_COMMON_ARGS}
        --quiet
        --error-exitcode=1
        ${COMMON_SOURCE_DIR}
        2> ./cppcheck-errors.txt
    )

    string(REPLACE ";" " " CPPCHECK_ARGS_STR "${CPPCHECK_ARGS}")
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK_EXE} --version
        COMMAND ${CPPCHECK_EXE} ${CPPCHECK_ARGS}
        COMMENT "running ${CPPCHECK_EXE} ${CPPCHECK_ARGS_STR}"
    )

    if(NOT CPPCHECK_HTMLREPORT_EXE-NOTFOUND)
        list(APPEND CPPCHECK_XML_ARGS
            ${CPPCHECK_COMMON_ARGS}
            --error-exitcode=0
            --xml
            ${COMMON_SOURCE_DIR}
            2> ./cppcheck-errors.xml
        )

        string(REPLACE ";" " " CPPCHECK_XML_ARGS_STR "${CPPCHECK_XML_ARGS}")
        add_custom_command(
            OUTPUT cppcheck-errors.xml
            COMMAND ${CPPCHECK_EXE} "--version"
            COMMAND ${CPPCHECK_EXE} ${CPPCHECK_XML_ARGS}
            COMMENT "running ${CPPCHECK_EXE} ${CPPCHECK_XML_ARGS_STR}"
        )

        list(APPEND CPPCHECK_HTMLREPORT_ARGS
            --file cppcheck-errors.xml
            --report-dir=cppcheck_report
            --source-dir=${CMAKE_SOURCE_DIR}
        )

        string(REPLACE ";" " " CPPCHECK_HTMLREPORT_ARGS_STR "${CPPCHECK_HTMLREPORT_ARGS}")
        add_custom_target(
            cppcheck-report
            COMMAND ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS}
            DEPENDS cppcheck-errors.xml
            COMMENT "running ${CPPCHECK_HTMLREPORT_EXE} ${CPPCHECK_HTMLREPORT_ARGS_STR}"
        )
    endif()
endif()
