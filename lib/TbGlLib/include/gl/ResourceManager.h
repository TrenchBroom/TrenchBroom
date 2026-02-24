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

#include "Notifier.h"
#include "gl/Resource.h"
#include "gl/ResourceId.h"

#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <ranges>
#include <vector>

namespace tb::gl
{

class ResourceWrapperBase
{
public:
  virtual ~ResourceWrapperBase() = default;

  virtual const ResourceId& id() const = 0;

  virtual long useCount() const = 0;

  virtual bool isDropped() const = 0;
  virtual bool needsProcessing() const = 0;

  virtual void drop() = 0;
  virtual bool process(TaskRunner taskRunner, const ProcessContext& processContext) = 0;
};

template <typename T>
class ResourceWrapper : public ResourceWrapperBase
{
private:
  std::shared_ptr<Resource<T>> m_resource;

  kdl_reflect_inline(ResourceWrapper, m_resource);

public:
  explicit ResourceWrapper(std::shared_ptr<Resource<T>> resource)
    : m_resource{std::move(resource)}
  {
  }

  const ResourceId& id() const override { return m_resource->id(); }
  long useCount() const override { return m_resource.use_count(); }
  bool isDropped() const override { return m_resource->isDropped(); }
  bool needsProcessing() const override { return m_resource->needsProcessing(); }
  void drop() override { m_resource->drop(); }
  bool process(TaskRunner taskRunner, const ProcessContext& processContext) override
  {
    return m_resource->process(taskRunner, processContext);
  }
  std::optional<std::string> error() const
  {
    return std::visit(
      kdl::overload(
        [](const ResourceFailed& failed) { return std::optional{failed.error}; },
        [](const auto&) { return std::nullopt; }),
      m_resource->state());
  };
};

class ResourceManager
{
public:
  Notifier<const std::vector<ResourceId>&> resourcesWereProcessedNotifier;

private:
  std::vector<std::unique_ptr<ResourceWrapperBase>> m_resources;

public:
  bool needsProcessing() const
  {
    return std::ranges::any_of(m_resources, [](const auto& resourceWrapper) {
      return resourceWrapper->useCount() == 1 || resourceWrapper->needsProcessing();
    });
  }

  std::vector<const ResourceWrapperBase*> resources() const
  {
    return m_resources | std::views::transform([](const auto& resourceWrapper) {
             return static_cast<const ResourceWrapperBase*>(resourceWrapper.get());
           })
           | kdl::ranges::to<std::vector>();
  }

  template <typename ResourceT>
  void addResource(std::shared_ptr<Resource<ResourceT>> resource)
  {
    m_resources.push_back(
      std::make_unique<ResourceWrapper<ResourceT>>(std::move(resource)));
  }

  void process(
    TaskRunner taskRunner,
    const ProcessContext& processContext,
    std::optional<std::chrono::milliseconds> timeout = std::nullopt)
  {
    const auto checkTimeout =
      timeout ? std::function{[timeout_ = *timeout,
                               startTime = std::chrono::steady_clock::now()]() {
        return std::chrono::steady_clock::now() - startTime < timeout_;
      }}
              : std::function{[]() { return true; }};

    auto processedResourceIds = std::vector<ResourceId>{};

    for (auto it = m_resources.begin(); it != m_resources.end() && checkTimeout();)
    {
      auto& resourceWrapper = *it;
      if (resourceWrapper->useCount() == 1 && !resourceWrapper->isDropped())
      {
        resourceWrapper->drop();
      }

      if (resourceWrapper->needsProcessing())
      {
        if (resourceWrapper->process(taskRunner, processContext))
        {
          processedResourceIds.push_back(resourceWrapper->id());
        }
      }

      it = resourceWrapper->useCount() == 1 && resourceWrapper->isDropped()
             ? m_resources.erase(it)
             : std::next(it);
    }

    if (!processedResourceIds.empty())
    {
      resourcesWereProcessedNotifier(processedResourceIds);
    }
  }
};

} // namespace tb::gl
