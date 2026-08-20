# Determines which sanitizer flags to enable, shared between cmake/CompilerConfig.cmake (applied
# to our own code via the CompilerConfig target) and cmake/dependencies/CompilerConfig.cmake's
# apply_sanitizer_options() (applied to third-party targets). Both need the same flags: if
# dependencies end up without sanitizer instrumentation while our own code has it, that mismatch
# makes AddressSanitizer's container-overflow check false-positive on containers shared across the
# boundary (see https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow).

if(TB_ENABLE_ASAN)
  message(STATUS "Enabling ASan")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND TB_SANITIZER_COMPILE_OPTIONS -fsanitize=address)
    list(APPEND TB_SANITIZER_LINK_OPTIONS -fsanitize=address)
  else()
    message(WARNING "TB isn't set up to enable ASan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

if(TB_ENABLE_TSAN)
  message(STATUS "Enabling TSan")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND TB_SANITIZER_COMPILE_OPTIONS -fsanitize=thread)
    list(APPEND TB_SANITIZER_LINK_OPTIONS -fsanitize=thread)
  else()
    message(WARNING "TB isn't set up to enable TSan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

if(TB_ENABLE_UBSAN)
  message(STATUS "Enabling UBSan")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND TB_SANITIZER_COMPILE_OPTIONS -fsanitize=undefined)
    list(APPEND TB_SANITIZER_LINK_OPTIONS -fsanitize=undefined)
  else()
    message(WARNING "TB isn't set up to enable UBSan for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()
