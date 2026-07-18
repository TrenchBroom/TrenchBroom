# assimp unconditionally appends "/Zi" to CMAKE_CXX_FLAGS_DEBUG for MSVC
# builds (see CMakeLists.txt's ELSEIF(MSVC) branch), which conflicts with the
# debug-info format this project applies to all fetched dependencies (see
# CMAKE_MSVC_DEBUG_INFORMATION_FORMAT in the top-level CMakeLists.txt),
# producing a harmless but noisy "cl : Command line warning D9025" message.
# Strip the "/Zi" from that line; /D_DEBUG and /Od are kept.
file(READ "CMakeLists.txt" _tb_assimp_cmakelists)
string(REPLACE "/D_DEBUG /Zi /Od" "/D_DEBUG /Od" _tb_assimp_cmakelists "${_tb_assimp_cmakelists}")
file(WRITE "CMakeLists.txt" "${_tb_assimp_cmakelists}")
