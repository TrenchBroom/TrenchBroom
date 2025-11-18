set(LIB_UPDATE_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_TEST_SOURCE
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestHttpClient.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestHttpClient.h"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestUtils.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestUtils.h"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestVersion.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/TestVersion.h"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/tst_GithubApi.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/tst_QtHttpClient.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/tst_Unzip.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/tst_UpdateController.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

add_executable(UpdateLibTest ${LIB_UPDATE_TEST_SOURCE})
target_include_directories(UpdateLibTest PRIVATE ${LIB_UPDATE_TEST_SOURCE_DIR} ${LIB_UPDATE_SOURCE_DIR})
target_include_directories(UpdateLibTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
target_link_libraries(UpdateLibTest PRIVATE UpdateLib Catch2::Catch2WithMain Qt6::Test)
set_target_properties(UpdateLibTest PROPERTIES AUTOMOC TRUE)

target_link_libraries(UpdateLibTest PRIVATE CompilerConfig)

# By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
# but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
# set_target_properties(UpdateLibTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:UpdateLibTest>")

# Copy fixtures
add_custom_command(
  TARGET UpdateLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:UpdateLibTest>/fixture"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/fixture" "$<TARGET_FILE_DIR:UpdateLibTest>/fixture"
)
