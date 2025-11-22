add_executable(TbFsLibTest)

target_sources(TbFsLibTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DiskFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_DiskIO.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_FileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_ImageFileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Reader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tst_VirtualFileSystem.cpp
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(TbFsLibTest PRIVATE CompilerConfig PrecompileStdHeaders)
target_link_libraries(TbFsLibTest PRIVATE Catch2::Catch2WithMain TbBaseTestUtilsLib TbFsLib TbFsTestUtilsLib)

# Copy fixtures
set(TEST_FIXTURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixture)
set(TEST_FIXTURE_DEST_DIR $<TARGET_FILE_DIR:TbFsLibTest>/fixture)

add_custom_command(TARGET TbFsLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_FIXTURE_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEST_FIXTURE_SOURCE_DIR}" "${TEST_FIXTURE_DEST_DIR}/test"
)
