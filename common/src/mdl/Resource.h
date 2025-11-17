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

#include "Macros.h"
#include "ResourceId.h"
#include "Result.h"

#include "kd/overload.h"
#include "kd/reflection_impl.h"
#include "kd/result.h"

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <variant>

namespace tb::mdl
{

template <typename T>
using ResourceLoader = std::function<Result<T>()>;

using ErrorHandler = std::function<void(const ResourceId&, const std::string&)>;

struct ProcessContext
{
  bool glContextAvailable;
  ErrorHandler errorHandler;
};

class TaskResult
{
};

template <typename T>
class LoaderTaskResult : public TaskResult
{
private:
  Result<T> m_result;

public:
  explicit LoaderTaskResult(Result<T> result)
    : m_result{std::move(result)}
  {
  }

  Result<T>&& get() { return std::move(m_result); }
};

using Task = std::function<std::unique_ptr<TaskResult>()>;
using TaskRunner = std::function<std::future<std::unique_ptr<TaskResult>>(Task)>;

template <typename T>
struct ResourceUnloaded
{
  ResourceLoader<T> loader;

  kdl_reflect_inline_empty(ResourceUnloaded);
};

template <typename T>
struct ResourceLoading
{
  std::future<std::unique_ptr<TaskResult>> future;

  kdl_reflect_inline_empty(ResourceLoading);
};

template <typename T>
struct ResourceLoaded
{
  T resource;

  kdl_reflect_inline(ResourceLoaded, resource);
};

template <typename T>
struct ResourceReady
{
  T resource;

  kdl_reflect_inline(ResourceReady, resource);
};

template <typename T>
struct ResourceDropping
{
  T resource;

  kdl_reflect_inline(ResourceDropping, resource);
};

struct ResourceDropped
{
  kdl_reflect_inline_empty(ResourceDropped);
};

struct ResourceFailed
{
  std::string error;

  kdl_reflect_inline(ResourceFailed, error);
};

template <typename T>
using ResourceState = std::variant<
  ResourceUnloaded<T>,
  ResourceLoading<T>,
  ResourceLoaded<T>,
  ResourceReady<T>,
  ResourceDropping<T>,
  ResourceDropped,
  ResourceFailed>;

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const ResourceState<T>& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

namespace detail
{

template <typename T>
ResourceState<T> triggerLoading(ResourceUnloaded<T> state, TaskRunner taskRunner)
{
  auto future = taskRunner([loader = std::move(state.loader)]() {
    return std::make_unique<LoaderTaskResult<T>>(loader());
  });
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

    auto taskResult = state.future.get();
    auto loaderTaskResult = static_cast<LoaderTaskResult<T>*>(taskResult.get());

    return std::move(loaderTaskResult->get())
           | kdl::transform([](auto value) -> ResourceState<T> {
               return ResourceLoaded<T>{std::move(value)};
             })
           | kdl::transform_error([](auto error) -> ResourceState<T> {
               return ResourceFailed{std::move(error.msg)};
             })
           | kdl::value();
  }
  return state;
}

template <typename T>
ResourceState<T> upload(ResourceLoaded<T> state, const bool glContextAvailable)
{
  state.resource.upload(glContextAvailable);
  return ResourceReady<T>{std::move(state.resource)};
}

template <typename T>
ResourceState<T> triggerDropping(ResourceReady<T> state)
{
  return ResourceDropping<T>{std::move(state.resource)};
}

template <typename T>
ResourceState<T> drop(ResourceReady<T> state, const bool glContextAvailable)
{
  state.resource.drop(glContextAvailable);
  return ResourceDropped{};
}

template <typename T>
ResourceState<T> drop(ResourceDropping<T> state, const bool glContextAvailable)
{
  state.resource.drop(glContextAvailable);
  return ResourceDropped{};
}

} // namespace detail

/**
 * A resource that can be loaded, uploaded, and dropped.
 *
 * The following table shows the state transitions of a resource:
 *
 * | State          | Transition       | New state       |
 * |----------------|------------------|-----------------|
 * | Unloaded       | process          | Loading         |
 * | Loading        | process          | Loaded or Failed|
 * | Loaded         | process          | Ready           |
 * | Ready          | drop             | Dropping        |
 * | Dropping       | process          | Dropped         |
 * | Dropped        | -                | -               |
 * | Failed         | -                | -               |
 */
template <typename T>
class Resource
{
private:
  ResourceId m_id;
  ResourceState<T> m_state;

  kdl_reflect_inline(Resource, m_state);

public:
  explicit Resource(ResourceLoader<T> loader)
    : m_state(ResourceUnloaded<T>{std::move(loader)})
  {
  }

  explicit Resource(T resource)
    : m_state(ResourceLoaded<T>{std::move(resource)})
  {
  }

  moveOnly(Resource);

  const ResourceId& id() const { return m_id; }

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

  T* get()
  {
    return std::visit(
      kdl::overload(
        [](ResourceLoaded<T>& state) -> T* { return &state.resource; },
        [](ResourceReady<T>& state) -> T* { return &state.resource; },
        [](auto&) -> T* { return nullptr; }),
      m_state);
  }

  bool isDropped() const { return std::holds_alternative<ResourceDropped>(m_state); }

  bool needsProcessing() const
  {
    return !std::holds_alternative<ResourceReady<T>>(m_state)
           && !std::holds_alternative<ResourceFailed>(m_state);
  }

  bool process(TaskRunner taskRunner, const ProcessContext& context)
  {
    const auto previousStateIndex = m_state.index();
    m_state = std::visit(
      kdl::overload(
        [&](ResourceUnloaded<T> state) -> ResourceState<T> {
          return detail::triggerLoading(std::move(state), taskRunner);
        },
        [&](ResourceLoading<T> state) -> ResourceState<T> {
          return detail::finishLoading(std::move(state));
        },
        [&](ResourceLoaded<T> state) -> ResourceState<T> {
          return detail::upload(std::move(state), context.glContextAvailable);
        },
        [&](ResourceDropping<T> state) -> ResourceState<T> {
          return detail::drop(std::move(state), context.glContextAvailable);
        },
        [](auto state) -> ResourceState<T> { return state; }),
      std::move(m_state));

    if (previousStateIndex != m_state.index())
    {
      if (const auto* failedState = std::get_if<ResourceFailed>(&m_state))
      {
        context.errorHandler(m_id, failedState->error);
      }
      return true;
    }

    return false;
  }

  void drop()
  {
    m_state = std::visit(
      kdl::overload(
        [](ResourceLoaded<T>) -> ResourceState<T> { return ResourceDropped{}; },
        [](ResourceReady<T> state) -> ResourceState<T> {
          return detail::triggerDropping(std::move(state));
        },
        [&](ResourceDropping<T> state) -> ResourceState<T> { return state; },
        [](auto) -> ResourceState<T> { return ResourceDropped{}; }),
      std::move(m_state));
  }

  void loadSync()
  {
    m_state = std::visit(
      kdl::overload(
        [&](ResourceUnloaded<T> state) -> ResourceState<T> {
          return state.loader() | kdl::transform([](auto value) -> ResourceState<T> {
                   return ResourceLoaded<T>{std::move(value)};
                 })
                 | kdl::transform_error([](auto error) -> ResourceState<T> {
                     return ResourceFailed{std::move(error.msg)};
                   })
                 | kdl::value();
        },
        [](auto state) -> ResourceState<T> { return state; }),
      std::move(m_state));
  }

  void uploadSync(const bool glContextAvailable)
  {
    m_state = std::visit(
      kdl::overload(
        [&](ResourceLoaded<T> state) -> ResourceState<T> {
          return detail::upload(std::move(state), glContextAvailable);
        },
        [](auto state) -> ResourceState<T> { return state; }),
      std::move(m_state));
  }

  void dropSync(const bool glContextAvailable)
  {
    m_state = std::visit(
      kdl::overload(
        [&](ResourceReady<T> state) -> ResourceState<T> {
          return detail::drop(std::move(state), glContextAvailable);
        },
        [&](ResourceDropping<T> state) -> ResourceState<T> {
          return detail::drop(std::move(state), glContextAvailable);
        },
        [](auto) -> ResourceState<T> { return ResourceDropped{}; }),
      std::move(m_state));
  }
};

template <typename T>
Resource(T) -> Resource<T>;

template <typename T>
std::ostream& operator<<(std::ostream& lhs, const std::shared_ptr<Resource<T>>& rhs)
{
  if (rhs)
  {
    lhs << *rhs;
  }
  else
  {
    lhs << "nullptr";
  }
  return lhs;
}

} // namespace tb::mdl
