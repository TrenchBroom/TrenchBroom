/*
 Copyright (C) 2023 Kristian Duske

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

#include "Assets/Resource.h"
#include "Error.h"
#include "Result.h"

#include "kdl/result.h"

#include "Catch2.h"

namespace TrenchBroom::Assets
{
TEST_CASE("Resource")
{
  auto taskRunner = TaskRunner{std::launch::deferred};

  auto resource = Resource<int>{[]() { return 1; }};
  CHECK(resource.get() == nullptr);
  CHECK(std::holds_alternative<ResourceUnloaded<int>>(resource.state()));

  resource.process(taskRunner);
  CHECK(resource.get() == nullptr);
  CHECK(std::holds_alternative<ResourceLoading<int>>(resource.state()));

  resource.process(taskRunner);
  // CHECK(resource.get() != nullptr);
  // CHECK(*resource.get() == 1);
  CHECK(std::holds_alternative<ResourceLoading<int>>(resource.state()));
}
} // namespace TrenchBroom::Assets
