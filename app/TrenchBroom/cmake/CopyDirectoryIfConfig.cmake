if(NOT DEFINED TB_CONFIG)
  message(FATAL_ERROR "TB_CONFIG is required")
endif()

if(NOT DEFINED REQUIRED_CONFIG)
  message(FATAL_ERROR "REQUIRED_CONFIG is required")
endif()

if(NOT DEFINED SRC)
  message(FATAL_ERROR "SRC is required")
endif()

if(NOT DEFINED DST)
  message(FATAL_ERROR "DST is required")
endif()

if(TB_CONFIG STREQUAL REQUIRED_CONFIG)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC}" "${DST}"
    COMMAND_ERROR_IS_FATAL ANY
  )
endif()