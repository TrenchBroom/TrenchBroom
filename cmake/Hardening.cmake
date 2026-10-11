# Enables the standard library's hardening checks (bounds checks in operator[], front(),
# back(), dereferencing an empty optional, etc.), turning what would otherwise be silent
# undefined behavior into an immediate crash.
#
# Debug builds are always hardened. TB_ENABLE_STDLIB_HARDENING extends that to all other
# build types. This is the inverse of TB_ENABLE_LTO: CI sets it for PR and master builds,
# but not for release-tag builds (see CI-*.sh/.bat), so the test suite runs against the
# checks on every commit without changing the binaries we actually ship.
#
# The definitions are added at directory scope rather than to the CompilerConfig target so
# that they also apply to third-party dependencies (this file is included before
# cmake/Dependencies.cmake). Unoptimized builds emit the standard library's inline
# functions out of line, and with libstdc++ the linker would otherwise be free to resolve
# our own calls to an unchecked copy from a dependency's object files.
if(TB_ENABLE_STDLIB_HARDENING)
  message(STATUS "Enabling standard library hardening for all build types")
  set(TB_STDLIB_HARDENING_CONDITION 1)
else()
  set(TB_STDLIB_HARDENING_CONDITION "$<CONFIG:Debug>")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC's Debug runtime already enables checked iterators (_ITERATOR_DEBUG_LEVEL=2) by
  # default, so the macro is only needed when hardening is requested for the other build
  # types.
  if(TB_ENABLE_STDLIB_HARDENING)
    add_compile_definitions(_MSVC_STL_HARDENING=1)
  endif()
else()
  # libstdc++ and libc++ each ignore the other's macro, so there is no need to detect
  # which one is in use. libc++'s debug mode enables every check it has, some of which are
  # too expensive for an optimized build, so the other build types use its extensive mode
  # instead.
  add_compile_definitions(
    "$<${TB_STDLIB_HARDENING_CONDITION}:_GLIBCXX_ASSERTIONS>"
    "$<${TB_STDLIB_HARDENING_CONDITION}:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_$<IF:$<CONFIG:Debug>,DEBUG,EXTENSIVE>>"
  )
endif()
