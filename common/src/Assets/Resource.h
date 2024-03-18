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

#include "Error.h"
#include "Result.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"
#include "kdl/result.h"

#include <functional>
#include <future>
#include <iosfwd>
#include <variant>

namespace TrenchBroom::Assets
{

template <typename T>
using Task = std::function<Result<T>()>;

class TaskRunner
{
private:
  std::launch m_launchPolicy;

public:
  explicit TaskRunner(std::launch launchPolicy);

  template <typename T>
  std::future<Result<T>> run(Task<T> task)
  {
    return std::async(m_launchPolicy, [task = std::move(task)]() { return task(); });
  }
};

template <typename T>
using ResourceLoader = std::function<Result<T>()>;

template <typename T>
struct ResourceUnloaded
{
  ResourceLoader<T> loader;
};

template <typename T>
struct ResourceLoading
{
  std::future<Result<T>> future;
};

template <typename T>
struct ResourceLoaded
{
  T resource;
};

template <typename T>
struct ResourceReady
{
  T resource;
};

struct ResourceDropped
{
};

struct ResourceFailed
{
  std::string error;
};

template <typename T>
using ResourceState = std::variant<
  ResourceUnloaded<T>,
  ResourceLoading<T>,
  ResourceLoaded<T>,
  ResourceReady<T>,
  ResourceDropped,
  ResourceFailed>;

namespace detail
{

template <typename T>
ResourceState<T> triggerLoading(ResourceUnloaded<T> state, TaskRunner& taskRunner)
{
  auto future = taskRunner.run(std::move(state.loader));
  return ResourceLoading<T>{std::move(future)};
}

template <typename T>
ResourceState<T> finishLoading(ResourceLoading<T> state)
{
  if (state.future.wait_for(std::chrono::seconds{0}) == std::future_status::ready)
  {
    if (!state.future.valid())
    {
      return ResourceFailed{"Invalid future"};
    }
    return state.future.get()
      .transform([](auto value) -> ResourceState<T> {
        return ResourceLoaded<T>{std::move(value)};
      })
      .transform_error([](auto error) -> ResourceState<T> {
        return ResourceFailed{std::move(error.msg)};
      })
      .value();
  }
  return std::move(state);
}

template <typename T>
ResourceState<T> upload(ResourceLoaded<T> state)
{
  // do something
  return ResourceReady<T>{std::move(state.resource)};
}

} // namespace detail

class ResourceBase
{
public:
  virtual ~ResourceBase() = default;

  virtual void process(TaskRunner& taskRunner) = 0;
  virtual void drop() = 0;
};

template <typename T>
class Resource : public ResourceBase
{
private:
  ResourceState<T> m_state;

  kdl_reflect_inline(Resource, m_state);

public:
  explicit Resource(ResourceLoader<T> loader)
    : m_state(ResourceUnloaded<T>{std::move(loader)})
  {
  }

  const ResourceState<T>& state() const { return m_state; }

  const T* get() const
  {
    return std::visit(
      kdl::overload(
        [](const ResourceLoaded<T>& state) -> const T* { return &state.resource; },
        [](const ResourceReady<T>& state) -> const T* { return &state.resource; },
        [](const auto&) -> const T* { return nullptr; }),
      m_state);
  }

  void process(TaskRunner& taskRunner) override
  {
    m_state = std::visit(
      kdl::overload(
        [&](ResourceUnloaded<T> state) -> ResourceState<T> {
          return detail::triggerLoading(std::move(state), taskRunner);
        },
        [&](ResourceLoading<T> state) -> ResourceState<T> {
          return detail::finishLoading(std::move(state));
        },
        [&](ResourceLoaded<T> state) -> ResourceState<T> {
          return detail::upload(std::move(state));
        },
        [](auto state) -> ResourceState<T> { return std::move(state); }),
      std::move(m_state));
  }

  void drop() override
  {
    m_state = std::visit(
      kdl::overload(
        [](ResourceLoaded<T>) { return ResourceDropped{}; },
        [](ResourceReady<T>) { return ResourceDropped{}; },
        [](auto) { return ResourceDropped{}; }),
      std::move(m_state));
  }
};

} // namespace TrenchBroom::Assets
