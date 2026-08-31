CPMAddPackage(
  URI "gh:leethomason/tinyxml2#9.0.0"
  OPTIONS "tinyxml2_BUILD_TESTING OFF"
)

suppress_dependency_warnings(tinyxml2)
apply_sanitizer_options(tinyxml2)
