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

#pragma once

#include "kdl/ranges/to.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <ranges>
#include <thread>
#include <vector>

namespace kdl
{

class task_manager
{
private:
  using pending_task = std::function<void()>;

  std::vector<std::thread> m_workers;

  std::mutex m_pending_tasks_mutex;
  std::condition_variable m_pending_tasks_cv;
  std::queue<pending_task> m_pending_tasks;
  bool m_running = true;

  std::function<void()> make_worker_func();

public:
  explicit task_manager(
    std::size_t max_concurrent_tasks = std::thread::hardware_concurrency());

  ~task_manager();

  template <typename task_result>
  auto run_task(std::function<task_result()> task)
  {
    if (m_workers.empty())
    {
      auto promise = std::promise<task_result>{};
      promise.set_value(task());
      return promise.get_future();
    }

    auto promise = std::make_shared<std::promise<task_result>>();
    auto future = promise->get_future();

    {
      auto lock = std::lock_guard{m_pending_tasks_mutex};
      m_pending_tasks.push([&, task_ = std::move(task), promise_ = std::move(promise)]() {
        promise_->set_value(task_());
      });
    }
    m_pending_tasks_cv.notify_one();

    return future;
  }

  template <std::ranges::range range>
  auto run_tasks(range tasks)
  {
    return tasks | std::views::transform([&](auto&& task) {
             return run_task(std::forward<decltype(task)>(task));
           })
           | kdl::ranges::to<std::vector>();
  }

  template <std::ranges::range range>
  auto run_tasks_and_wait(range&& tasks)
  {
    auto futures = run_tasks(std::forward<range>(tasks));
    return futures | std::views::transform([](auto& future) { return future.get(); })
           | kdl::ranges::to<std::vector>();
  }
};

} // namespace kdl
