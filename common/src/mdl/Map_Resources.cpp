/*
 Copyright (C) 2025 Kristian Duske

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

#include "Map.h"
#include "mdl/ResourceManager.h"

#include "kdl/task_manager.h"

#include <memory>

namespace tb::mdl
{

void Map::processResourcesSync(const ProcessContext& processContext)
{
  auto allProcessedResourceIds = std::vector<ResourceId>{};
  while (m_resourceManager->needsProcessing())
  {
    auto processedResourceIds = m_resourceManager->process(
      [](auto task) {
        auto promise = std::promise<std::unique_ptr<TaskResult>>{};
        promise.set_value(task());
        return promise.get_future();
      },
      processContext);

    allProcessedResourceIds = kdl::vec_concat(
      std::move(allProcessedResourceIds), std::move(processedResourceIds));
  }

  if (!allProcessedResourceIds.empty())
  {
    resourcesWereProcessedNotifier.notify(
      kdl::vec_sort_and_remove_duplicates(std::move(allProcessedResourceIds)));
  }
}

void Map::processResourcesAsync(const ProcessContext& processContext)
{
  using namespace std::chrono_literals;

  const auto processedResourceIds = m_resourceManager->process(
    [&](auto task) { return m_taskManager.run_task(std::move(task)); },
    processContext,
    20ms);

  if (!processedResourceIds.empty())
  {
    resourcesWereProcessedNotifier.notify(processedResourceIds);
  }
}

bool Map::needsResourceProcessing() const
{
  return m_resourceManager->needsProcessing();
}

} // namespace tb::mdl
