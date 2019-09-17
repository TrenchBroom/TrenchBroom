set(COMMON_BENCHMARK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_BENCHMARK_SOURCE
        "${COMMON_BENCHMARK_SOURCE_DIR}/BenchmarkUtils.h"
        "${COMMON_BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.h"
        "${COMMON_BENCHMARK_SOURCE_DIR}/AABBTreeBenchmark.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/IO/TestParserStatus.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/Main.cpp"
        "${COMMON_BENCHMARK_SOURCE_DIR}/Renderer/BrushRendererBenchmark.cpp"
)

add_executable(common-benchmark ${COMMON_BENCHMARK_SOURCE})
target_include_directories(common-benchmark PRIVATE ${COMMON_BENCHMARK_SOURCE_DIR})
target_link_libraries(common-benchmark PRIVATE common gtest)

set(BENCHMARK_FIXTURE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixture")

set(BENCHMARK_RESOURCE_DEST_DIR "$<TARGET_FILE_DIR:common-benchmark>")
set(BENCHMARK_FIXTURE_DEST_DIR "${BENCHMARK_RESOURCE_DEST_DIR}/fixture")

# Clear all fixtures
ADD_CUSTOM_COMMAND(TARGET common-benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${BENCHMARK_FIXTURE_DEST_DIR}")

# Copy test fixtures
ADD_CUSTOM_COMMAND(TARGET common-benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BENCHMARK_FIXTURE_SOURCE_DIR}" "${BENCHMARK_FIXTURE_DEST_DIR}/benchmark")
