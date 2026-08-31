# fast_float declares an old cmake_minimum_required, which triggers a deprecation
# warning under CMake 4. Use >= 3.13 so CMP0077 is NEW and the FASTFLOAT_TEST
# option below overrides fast_float's option() default without a warning.
set(CMAKE_POLICY_VERSION_MINIMUM 3.13)
CPMAddPackage(
  URI "gh:fastfloat/fast_float#v6.1.4"
  OPTIONS "FASTFLOAT_TEST OFF"
)
unset(CMAKE_POLICY_VERSION_MINIMUM)
