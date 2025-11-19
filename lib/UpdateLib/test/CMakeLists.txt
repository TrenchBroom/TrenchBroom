add_executable(UpdateLibTest)
target_sources(UpdateLibTest PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestHttpClient.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestHttpClient.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUtils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestVersion.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/TestVersion.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_GithubApi.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_QtHttpClient.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_Unzip.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tst_UpdateController.cpp"
)

add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1)

target_link_libraries(UpdateLibTest PRIVATE CompilerConfig)
target_link_libraries(UpdateLibTest PRIVATE UpdateLib Catch2::Catch2WithMain Qt6::Test)

set_target_properties(UpdateLibTest PROPERTIES AUTOMOC TRUE)

# Copy fixtures
add_custom_command(
  TARGET UpdateLibTest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:UpdateLibTest>/fixture"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/fixture" "$<TARGET_FILE_DIR:UpdateLibTest>/fixture"
)
