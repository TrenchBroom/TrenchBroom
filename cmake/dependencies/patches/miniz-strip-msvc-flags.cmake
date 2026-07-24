# miniz unconditionally appends "/W3 /Zi /permissive-" to CMAKE_C_FLAGS for
# MSVC builds, even when built as a subproject. This conflicts with the
# warning level and debug-info format this project applies to all fetched
# dependencies (see suppress_dependency_warnings and
# CMAKE_MSVC_DEBUG_INFORMATION_FORMAT), producing harmless but noisy
# "cl : Command line warning D9025" messages. Strip miniz's hardcoded flags
# here; /permissive- doesn't conflict with anything, so it is kept.
file(READ "CMakeLists.txt" _tb_miniz_cmakelists)
string(REPLACE "/W3 /Zi /permissive-" "/permissive-" _tb_miniz_cmakelists "${_tb_miniz_cmakelists}")
file(WRITE "CMakeLists.txt" "${_tb_miniz_cmakelists}")
