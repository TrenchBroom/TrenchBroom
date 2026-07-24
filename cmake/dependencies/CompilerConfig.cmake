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
