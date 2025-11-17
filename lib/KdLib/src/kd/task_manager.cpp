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

#include "kd/task_manager.h"

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

std::function<void()> task_manager::make_worker_func()
{
  return [&] {
    while (true)
    {
      auto lock = std::unique_lock{m_pending_tasks_mutex};
      m_pending_tasks_cv.wait(
        lock, [&] { return !m_running || !m_pending_tasks.empty(); });

      if (!m_running)
      {
        break;
      }

      if (!m_pending_tasks.empty())
      {
        auto task = std::move(m_pending_tasks.front());
        m_pending_tasks.pop();
        lock.unlock();

        task();
      }
    }
  };
}

task_manager::task_manager(const std::size_t max_concurrent_tasks)
{
  for (size_t i = 0; i < max_concurrent_tasks; ++i)
  {
    m_workers.emplace_back(make_worker_func());
  }
}

task_manager::~task_manager()
{
  {
    auto lock = std::lock_guard{m_pending_tasks_mutex};
    m_running = false;
  }

  m_pending_tasks_cv.notify_all();
  for (auto& worker : m_workers)
  {
    worker.join();
  }
}

} // namespace kdl
