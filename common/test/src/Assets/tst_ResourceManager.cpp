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

#include "Assets/MockTaskRunner.h"
#include "Assets/Resource.h"
#include "Assets/ResourceManager.h"
#include "Error.h"
#include "Result.h"

#include "kdl/reflection_impl.h"
#include "kdl/vector_utils.h"

#include "Catch2.h"

namespace TrenchBroom::Assets
{
namespace
{

struct MockResource
{
  void upload(const bool glContextAvailable) const { mockUpload(glContextAvailable); }
  void drop(const bool glContextAvailable) const { mockDrop(glContextAvailable); }

  std::function<void(bool)> mockUpload = [](auto) {};
  std::function<void(bool)> mockDrop = [](auto) {};

  kdl_reflect_inline_empty(MockResource);
};

using ResourceT = Resource<MockResource>;
using ResourceWrapperT = ResourceWrapper<MockResource>;

bool operator==(
  const std::vector<const ResourceWrapperBase*>& lhs,
  const std::vector<std::shared_ptr<ResourceT>>& rhs)
{
  if (lhs.size() != rhs.size())
  {
    return false;
  }

  for (size_t i = 0; i < lhs.size(); ++i)
  {
    const auto* lhsCast = dynamic_cast<const ResourceWrapperT*>(lhs[i]);

    if (!lhsCast || *lhsCast != ResourceWrapperT{rhs[i]})
    {
      return false;
    }
  }

  return true;
}

} // namespace

TEST_CASE("ResourceManager")
{
  const auto mockResourceLoader = [&]() { return Result<MockResource>{MockResource{}}; };

  auto mockTaskRunner = MockTaskRunner{};
  auto taskRunner = [&](auto task) { return mockTaskRunner.run(std::move(task)); };

  const auto glContextAvailable = GENERATE(true, false);
  const auto processContext = ProcessContext{glContextAvailable};

  auto resourceManager = ResourceManager{};

  SECTION("needsProcessing")
  {
    CHECK(!resourceManager.needsProcessing());

    auto resource1 = std::make_shared<ResourceT>(mockResourceLoader);
    resourceManager.addResource(resource1);

    REQUIRE(std::holds_alternative<ResourceUnloaded<MockResource>>(resource1->state()));
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceLoading<MockResource>>(resource1->state()));
    CHECK(resourceManager.needsProcessing());

    mockTaskRunner.resolveNextPromise();
    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceLoaded<MockResource>>(resource1->state()));
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
    CHECK(!resourceManager.needsProcessing());

    auto resource2 = std::make_shared<ResourceT>(mockResourceLoader);
    resourceManager.addResource(resource2);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
    REQUIRE(std::holds_alternative<ResourceUnloaded<MockResource>>(resource2->state()));
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
    REQUIRE(std::holds_alternative<ResourceLoading<MockResource>>(resource2->state()));
    CHECK(resourceManager.needsProcessing());

    mockTaskRunner.resolveNextPromise();
    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
    REQUIRE(std::holds_alternative<ResourceLoaded<MockResource>>(resource2->state()));
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
    CHECK(!resourceManager.needsProcessing());

    resource1.reset();
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
    CHECK(!resourceManager.needsProcessing());

    resource2.reset();
    CHECK(resourceManager.needsProcessing());

    resourceManager.process(taskRunner, processContext);
    CHECK(!resourceManager.needsProcessing());
  }

  SECTION("addResource")
  {
    auto resource1 = std::make_shared<ResourceT>(mockResourceLoader);
    resourceManager.addResource(resource1);

    CHECK(resourceManager.resources() == std::vector{resource1});
    CHECK(resource1.use_count() == 2);
    CHECK(std::holds_alternative<ResourceUnloaded<MockResource>>(resource1->state()));

    auto resource2 = std::make_shared<ResourceT>(mockResourceLoader);
    resourceManager.addResource(resource2);

    CHECK(resourceManager.resources() == std::vector{resource1, resource2});
  }

  SECTION("process")
  {
    SECTION("resource loading")
    {
      auto resource1 = std::make_shared<ResourceT>(mockResourceLoader);
      auto resource2 = std::make_shared<ResourceT>(mockResourceLoader);
      resourceManager.addResource(resource1);
      resourceManager.addResource(resource2);

      CHECK(
        resourceManager.process(taskRunner, processContext)
        == std::vector{resource1->id(), resource2->id()});
      CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource1->state()));
      CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource2->state()));

      SECTION("resource1 finishes loading")
      {
        mockTaskRunner.resolveNextPromise();

        CHECK(
          resourceManager.process(taskRunner, processContext)
          == std::vector{resource1->id()});
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource1->state()));
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource2->state()));

        SECTION("resource2 finishes loading")
        {
          mockTaskRunner.resolveNextPromise();

          CHECK(
            resourceManager.process(taskRunner, processContext)
            == std::vector{resource1->id(), resource2->id()});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource2->state()));

          CHECK(
            resourceManager.process(taskRunner, processContext)
            == std::vector{resource2->id()});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
        }
      }

      SECTION("resource2 finishes loading")
      {
        mockTaskRunner.resolveLastPromise();

        CHECK(
          resourceManager.process(taskRunner, processContext)
          == std::vector{resource2->id()});
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource1->state()));
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource2->state()));

        SECTION("resource1 finishes loading")
        {
          mockTaskRunner.resolveLastPromise();

          CHECK(
            resourceManager.process(taskRunner, processContext)
            == std::vector{resource1->id(), resource2->id()});
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));

          CHECK(
            resourceManager.process(taskRunner, processContext)
            == std::vector{resource1->id()});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
        }
      }
    }

    SECTION("dropping resources")
    {
      auto mockDropCalls = std::array{std::optional<bool>{}, std::optional<bool>{}};
      auto sharedResources = std::array{
        std::make_shared<ResourceT>([&]() {
          return Result<MockResource>{MockResource{
            [](auto) {},
            [&](const auto i_glContextAvailable) {
              mockDropCalls[0] = i_glContextAvailable;
            },
          }};
        }),
        std::make_shared<ResourceT>([&]() {
          return Result<MockResource>{MockResource{
            [](auto) {},
            [&](const auto i_glContextAvailable) {
              mockDropCalls[1] = i_glContextAvailable;
            },
          }};
        }),
      };

      const auto resourceIds = kdl::vec_transform(
        sharedResources, [](const auto& resource) { return resource->id(); });

      resourceManager.addResource(sharedResources[0]);
      resourceManager.addResource(sharedResources[1]);

      resourceManager.process(taskRunner, processContext);
      mockTaskRunner.resolveNextPromise();
      mockTaskRunner.resolveNextPromise();
      resourceManager.process(taskRunner, processContext);
      resourceManager.process(taskRunner, processContext);
      REQUIRE(
        std::holds_alternative<ResourceReady<MockResource>>(sharedResources[0]->state()));
      REQUIRE(
        std::holds_alternative<ResourceReady<MockResource>>(sharedResources[1]->state()));

      sharedResources[0].reset();
      CHECK(resourceManager.resources().size() == 2);

      CHECK(
        resourceManager.process(taskRunner, processContext)
        == std::vector{resourceIds[0]});
      CHECK(resourceManager.resources() == std::vector{sharedResources[1]});
      CHECK(mockDropCalls[0] == glContextAvailable);

      sharedResources[1].reset();
      CHECK(resourceManager.resources().size() == 1);

      CHECK(
        resourceManager.process(taskRunner, processContext)
        == std::vector{resourceIds[1]});
      CHECK(resourceManager.resources().empty());
      CHECK(mockDropCalls[1] == glContextAvailable);
    }
  }
}

} // namespace TrenchBroom::Assets
