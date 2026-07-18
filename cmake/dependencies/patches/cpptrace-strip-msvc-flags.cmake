# cpptrace unconditionally applies "/W4" to its own target via a genex in
# CMakeLists.txt's warning_options, which conflicts with the warning
# suppression this project applies to all fetched dependencies (see
# suppress_dependency_warnings), producing a harmless but noisy
# "cl : Command line warning D9025" message. Strip the "/W4"; /permissive- is
# kept since it doesn't conflict with anything.
file(READ "CMakeLists.txt" _tb_cpptrace_cmakelists)
string(REPLACE "MSVC>:/W4 /permissive->" "MSVC>:/permissive->" _tb_cpptrace_cmakelists "${_tb_cpptrace_cmakelists}")
file(WRITE "CMakeLists.txt" "${_tb_cpptrace_cmakelists}")
