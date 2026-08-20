function(suppress_dependency_warnings target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "Cannot suppress warnings for missing dependency target ${target}")
  endif()

  get_target_property(imported "${target}" IMPORTED)
  if(imported)
    return()
  endif()

  get_target_property(target_type "${target}" TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    return()
  endif()

  target_compile_options("${target}" PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang,GNU>:-w>
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-w>
    $<$<COMPILE_LANG_AND_ID:C,MSVC>:/w>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/w>
  )
endfunction()

# Applies the sanitizer flags determined by cmake/Sanitizers.cmake (the same ones applied to our
# own code via the CompilerConfig target) to a third-party target. Dependencies aren't linked
# against CompilerConfig, so without this they'd be compiled without sanitizer instrumentation
# while our own code has it; that mismatch makes AddressSanitizer's container-overflow check
# false-positive on containers shared across the boundary (e.g. inside Catch2's XmlWriter).
function(apply_sanitizer_options target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "Cannot apply sanitizer options to missing dependency target ${target}")
  endif()

  get_target_property(imported "${target}" IMPORTED)
  if(imported)
    return()
  endif()

  get_target_property(target_type "${target}" TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    return()
  endif()

  target_compile_options("${target}" PRIVATE ${TB_SANITIZER_COMPILE_OPTIONS})
  target_link_options("${target}" PRIVATE ${TB_SANITIZER_LINK_OPTIONS})
endfunction()
