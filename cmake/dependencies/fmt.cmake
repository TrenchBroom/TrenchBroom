CPMAddPackage(
  URI "gh:fmtlib/fmt#11.2.0"
  OPTIONS
    "FMT_DOC OFF"
    "FMT_TEST OFF"
)

suppress_dependency_warnings(fmt)
apply_sanitizer_options(fmt)
