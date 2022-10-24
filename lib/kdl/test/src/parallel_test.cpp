/*
 Copyright 2020 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kdl/parallel.h"

#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "test_utils.h"

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("for 0", "[parallel_test]")
{
  bool ran = false;
  kdl::parallel_for(0, [&](size_t) { ran = true; });
  CHECK(!ran);
}

TEST_CASE("for 10000", "[parallel_test]")
{
  constexpr size_t TestSize = 10'000;

  std::array<std::atomic<size_t>, TestSize> indices;
  for (size_t i = 0; i < TestSize; ++i)
  {
    indices[i] = 0;
  }

  // fill `indices` with 1, ..., TestSize
  bool failed = false;
  kdl::parallel_for(indices.size(), [&](const size_t i) {
    if (i >= TestSize)
    {
      failed = true;
      return;
    }
    const size_t oldValue = std::atomic_fetch_add(&indices[i], i + 1);
    if (oldValue != 0)
    {
      failed = true;
    }
  });

  CHECK(!failed);
  for (size_t i = 0; i < TestSize; ++i)
  {
    const size_t expected = (i + 1);
    CHECK(indices[i] == expected);
  }
}

TEST_CASE("transform", "[parallel_test]")
{
  const auto L = [](const int& v) { return v * 10; };

  CHECK(std::vector<int>{} == kdl::vec_parallel_transform(std::vector<int>{}, L));
  CHECK(
    std::vector<int>{10, 20, 30}
    == kdl::vec_parallel_transform(std::vector<int>{1, 2, 3}, L));
}

TEST_CASE("transform_many", "[parallel_test]")
{
  std::vector<int> input;
  std::vector<std::string> expected;

  for (int i = 0; i < 10000; ++i)
  {
    input.push_back(i);
    expected.push_back(std::to_string(i));
  }

  CHECK(expected == kdl::vec_parallel_transform(input, [](int i) {
          return std::to_string(i);
        }));
}

TEST_CASE("overhead for small work batches", "[parallel_test]")
{
  constexpr size_t OuterLoop = 1'000;
  constexpr size_t InnerLoop = 10;
  auto counter = std::atomic<size_t>{0};
  const auto startTime = std::chrono::high_resolution_clock::now();

  SECTION("sequential")
  {
    for (size_t i = 0; i < OuterLoop * InnerLoop; ++i)
    {
      std::atomic_fetch_add(&counter, static_cast<size_t>(1));
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "sequential took "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                   endTime - startTime)
                   .count()
              << "us\n";
  }

  SECTION("run parallel_for on batches of 10")
  {
    for (size_t i = 0; i < OuterLoop; ++i)
    {
      kdl::parallel_for(InnerLoop, [&](const size_t) {
        std::atomic_fetch_add(&counter, static_cast<size_t>(1));
      });
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "small batches took "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                   endTime - startTime)
                   .count()
              << "us\n";
  }

  CHECK(static_cast<size_t>(counter) == OuterLoop * InnerLoop);
}
} // namespace kdl
