set(RETURN_EXITCODE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(RETURN_EXITCODE_SOURCE
        "${RETURN_EXITCODE_SOURCE_DIR}/Main.cpp")
set(RETURN_EXITCODE_SOURCE ${RETURN_EXITCODE_SOURCE})

add_executable(return-exitcode ${RETURN_EXITCODE_SOURCE})
target_include_directories(return-exitcode PRIVATE ${RETURN_EXITCODE_SOURCE_DIR})

set_compiler_config(return-exitcode)
