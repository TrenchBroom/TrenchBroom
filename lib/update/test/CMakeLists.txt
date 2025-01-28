set(LIB_UPDATE_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_TEST_SOURCE
  "${LIB_UPDATE_TEST_SOURCE_DIR}/RunAllTests.cpp"
  "${LIB_UPDATE_TEST_SOURCE_DIR}/upd/tst_HttpApi.cpp"
)

# Catch2 needs one source file to define a macro (in our case CATCH_CONFIG_RUNNER) before including
# catch.hpp. This needs to be excluded from unity builds, otherwise the macro wouldn't be defined
# before the first catch.hpp inclusion.
set_property(SOURCE "${LIB_UPDATE_TEST_SOURCE_DIR}/RunAllTests.cpp" PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

add_executable(lib_update-test ${LIB_UPDATE_TEST_SOURCE})
target_include_directories(lib_update-test PRIVATE ${LIB_UPDATE_TEST_SOURCE_DIR} ${LIB_UPDATE_SOURCE_DIR})
target_include_directories(lib_update-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
target_link_libraries(lib_update-test PRIVATE lib_update Catch2::Catch2)
set_target_properties(lib_update-test PROPERTIES AUTOMOC TRUE)

set_compiler_config(lib_update-test)

# By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
# but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
set_target_properties(lib_update-test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:lib_update-test>")

if(WIN32)
  # Copy DLLs to app directory
  add_custom_command(TARGET lib_update-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:fmt::fmt>" "${TEST_RESOURCE_DEST_DIR}")
endif()
