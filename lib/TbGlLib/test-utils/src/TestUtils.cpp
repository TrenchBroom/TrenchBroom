/*
 Copyright (C) 2026 Kristian Duske

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

#include "gl/TestUtils.h"

#include "gl/ResourceManager.h"

namespace tb::gl
{

void processResourcesSync(
  ResourceManager& resourceManager, const ProcessContext& processContext)
{
  while (resourceManager.needsProcessing())
  {
    resourceManager.process(
      [](auto task) {
        auto promise = std::promise<std::unique_ptr<TaskResult>>{};
        promise.set_value(task());
        return promise.get_future();
      },
      processContext);
  }
}

} // namespace tb::gl