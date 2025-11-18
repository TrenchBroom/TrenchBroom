/*
 Copyright 2024 Kristian Duske

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

#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/task_manager.h"

#include <memory>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace kdl
{
namespace
{
template <typename result_type>
auto make_task(result_type&& result)
{
  using task_type = std::function<result_type()>;

  auto task_ran = std::make_shared<bool>(false);
  auto task = task_type{[task_ran, result_ = std::forward<result_type>(result)]() {
    *task_ran = true;
    return result_;
  }};

  return std::tuple<task_type, bool&>{std::move(task), *task_ran};
}
} // namespace

TEST_CASE("task_manager")
{
  using namespace std::string_literals;

  const auto max_concurrent_tasks = GENERATE(0u, 1u, 2u, 3u, 4u);
  CAPTURE(max_concurrent_tasks);

  auto tm = task_manager{max_concurrent_tasks};

  SECTION("run_task")
  {
    auto [task1, task_ran1] = make_task(4);
    auto [task2, task_ran2] = make_task("asdf"s);
    auto [task3, task_ran3] = make_task(15);

    REQUIRE(!task_ran1);
    REQUIRE(!task_ran2);
    REQUIRE(!task_ran3);

    auto future1 = tm.run_task(task1);
    auto future2 = tm.run_task(task2);
    REQUIRE(future1.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
    CHECK(task_ran1);
    CHECK(future1.get() == 4);

    REQUIRE(future2.wait_for(std::chrono::seconds{1}) == std::future_status::ready);
    CHECK(future2.get() == "asdf"s);
    CHECK(task_ran2);

    auto future3 = tm.run_task(task3);
    REQUIRE(future3.wait_for(std::chrono::seconds{1}) == std::future_status::ready);

    CHECK(task_ran3);
    CHECK(future3.get() == 15);
  }

  SECTION("run_tasks")
  {
    auto [task1, task_ran1] = make_task(4);
    auto [task2, task_ran2] = make_task(10);
    auto [task3, task_ran3] = make_task(15);

    REQUIRE(!task_ran1);
    REQUIRE(!task_ran2);
    REQUIRE(!task_ran3);

    auto futures = tm.run_tasks(std::vector{task1, task2, task3}) | kdl::views::as_rvalue
                   | kdl::ranges::to<std::vector>();

    CHECK(futures[0].get() == 4);
    CHECK(futures[1].get() == 10);
    CHECK(futures[2].get() == 15);


    CHECK(task_ran1);
    CHECK(task_ran2);
    CHECK(task_ran3);
  }

  SECTION("run_tasks_and_wait")
  {
    auto [task1, task_ran1] = make_task(4);
    auto [task2, task_ran2] = make_task(10);
    auto [task3, task_ran3] = make_task(15);

    REQUIRE(!task_ran1);
    REQUIRE(!task_ran2);
    REQUIRE(!task_ran3);

    CHECK(
      (tm.run_tasks_and_wait(std::vector{task1, task2, task3})
       | kdl::ranges::to<std::vector>())
      == std::vector{4, 10, 15});

    CHECK(task_ran1);
    CHECK(task_ran2);
    CHECK(task_ran3);
  }
}

TEST_CASE("task_manager stress test")
{
  auto tm = task_manager{};

  const auto ints = std::views::iota(0, 1000) | kdl::ranges::to<std::vector>();
  using diff_type = std::ranges::range_difference_t<decltype(ints)>;

  auto futures = std::vector<std::future<int>>{};

  for (auto i = ints.begin(); i < ints.end(); i += 100)
  {
    const auto e =
      i + std::min(static_cast<diff_type>(100), static_cast<diff_type>(ints.end() - i));

    auto tasks = std::ranges::subrange{i, e} | std::views::transform([&](int j) {
                   return std::function{[j] {
                     std::this_thread::sleep_for(std::chrono::milliseconds{100});
                     return j;
                   }};
                 })
                 | kdl::ranges::to<std::vector>();

    auto new_futures = tm.run_tasks(tasks);
    futures.insert(
      futures.end(),
      std::make_move_iterator(new_futures.begin()),
      std::make_move_iterator(new_futures.end()));
  }

  const auto results = futures
                       | std::views::transform([](auto& future) { return future.get(); })
                       | kdl::ranges::to<std::vector>();
  CHECK(results == results);
}

} // namespace kdl
