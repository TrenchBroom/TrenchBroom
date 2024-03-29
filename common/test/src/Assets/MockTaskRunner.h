/*
 Copyright (C) 2024 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Assets/Resource.h"

#include "kdl/vector_utils.h"

#include <future>
#include <memory>

namespace TrenchBroom::Assets
{

struct MockTaskRunner
{
  auto run(Task task)
  {
    auto promise = std::promise<std::unique_ptr<TaskResult>>{};
    auto future = promise.get_future();
    tasks.emplace_back(std::move(promise), std::move(task));
    return future;
  }

  void resolveNextPromise()
  {
    auto [promise, task] = kdl::vec_pop_front(tasks);
    promise.set_value(task());
  }

  void resolveLastPromise()
  {
    auto [promise, task] = kdl::vec_pop_back(tasks);
    promise.set_value(task());
  }

  std::vector<std::tuple<std::promise<std::unique_ptr<TaskResult>>, Task>> tasks;
};

} // namespace TrenchBroom::Assets