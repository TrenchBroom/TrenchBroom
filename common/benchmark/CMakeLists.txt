set(COMMON_BENCHMARK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_BENCHMARK_SOURCE
        "${COMMON_BENCHMARK_SOURCE_DIR}/BenchmarkUtils.h"
        "${COMMON_BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.h"
        "${COMMON_BENCHMARK_SOURCE_DIR}/AABBTreeBenchmark.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/Main.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/Renderer/BrushRendererBenchmark.cpp"
)

set_property(SOURCE "${COMMON_BENCHMARK_SOURCE_DIR}/Main.cpp" PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

add_executable(common-benchmark ${COMMON_BENCHMARK_SOURCE})
target_include_directories(common-benchmark PRIVATE ${COMMON_BENCHMARK_SOURCE_DIR})
target_link_libraries(common-benchmark PRIVATE common Catch2::Catch2)

set_compiler_config(common-benchmark)

# By default VS launches with a CWD one level up from the .exe (which is in a "Debug" subdirectory)
# but we copy resources into the .exe's directory, and the tests expect the CWD to be the .exe's directory.
set_target_properties(common-benchmark PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:common-benchmark>")

set(BENCHMARK_FIXTURE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixture")

set(BENCHMARK_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:common-benchmark>")
set(BENCHMARK_FIXTURE_DEST_DIR "${BENCHMARK_RESOURCE_DEST_DIR}/fixture")

if(WIN32)
    # Copy DLLs to app directory
    add_custom_command(TARGET common-benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeimage>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freetype>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:z>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Widgets>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Gui>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Core>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::Svg>" "$<TARGET_FILE_DIR:common-benchmark>"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:common-benchmark>/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory    "$<TARGET_FILE_DIR:common-benchmark>/styles"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" "$<TARGET_FILE_DIR:common-benchmark>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" "$<TARGET_FILE_DIR:common-benchmark>/styles")
endif()

# Clear all fixtures
add_custom_command(TARGET common-benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${BENCHMARK_FIXTURE_DEST_DIR}")

# Copy test fixtures
add_custom_command(TARGET common-benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BENCHMARK_FIXTURE_SOURCE_DIR}" "${BENCHMARK_FIXTURE_DEST_DIR}/benchmark")
