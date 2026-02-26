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

#include "Observer.h"
#include "Result.h"
#include "gl/MockTaskRunner.h"
#include "gl/Resource.h"
#include "gl/ResourceManager.h"
#include "gl/TestGl.h"

#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::gl
{
namespace
{

struct MockResource
{
  void upload(Gl& gl) const { mockUpload(gl); }
  void drop(Gl& gl) const { mockDrop(gl); }

  std::function<void(Gl&)> mockUpload = [](auto&) {};
  std::function<void(Gl&)> mockDrop = [](auto&) {};

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

  auto testGl = TestGl{};

  const auto processContext = ProcessContext{testGl, [](auto, auto) {}};

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
    auto resourcesWereProcessed =
      Observer<std::vector<ResourceId>>{resourceManager.resourcesWereProcessedNotifier};

    SECTION("resource loading")
    {
      auto resource1 = std::make_shared<ResourceT>(mockResourceLoader);
      auto resource2 = std::make_shared<ResourceT>(mockResourceLoader);
      resourceManager.addResource(resource1);
      resourceManager.addResource(resource2);

      resourceManager.process(taskRunner, processContext);
      CHECK(
        resourcesWereProcessed.notifications
        == std::vector<std::vector<ResourceId>>{{{resource1->id(), resource2->id()}}});
      CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource1->state()));
      CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource2->state()));

      SECTION("resource1 finishes loading")
      {
        resourcesWereProcessed.reset();
        mockTaskRunner.resolveNextPromise();
        resourceManager.process(taskRunner, processContext);

        CHECK(
          resourcesWereProcessed.notifications
          == std::vector<std::vector<ResourceId>>{{resource1->id()}});
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource1->state()));
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource2->state()));

        SECTION("resource2 finishes loading")
        {
          resourcesWereProcessed.reset();
          mockTaskRunner.resolveNextPromise();
          resourceManager.process(taskRunner, processContext);

          CHECK(
            resourcesWereProcessed.notifications
            == std::vector<std::vector<ResourceId>>{{resource1->id(), resource2->id()}});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource2->state()));

          resourcesWereProcessed.reset();
          resourceManager.process(taskRunner, processContext);

          CHECK(
            resourcesWereProcessed.notifications
            == std::vector<std::vector<ResourceId>>{{resource2->id()}});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
        }
      }

      SECTION("resource2 finishes loading")
      {
        resourcesWereProcessed.reset();
        mockTaskRunner.resolveLastPromise();
        resourceManager.process(taskRunner, processContext);

        CHECK(
          resourcesWereProcessed.notifications
          == std::vector<std::vector<ResourceId>>{{resource2->id()}});
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource1->state()));
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource2->state()));

        SECTION("resource1 finishes loading")
        {
          resourcesWereProcessed.reset();
          mockTaskRunner.resolveLastPromise();
          resourceManager.process(taskRunner, processContext);

          CHECK(
            resourcesWereProcessed.notifications
            == std::vector<std::vector<ResourceId>>{{resource1->id(), resource2->id()}});
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));

          resourcesWereProcessed.reset();
          resourceManager.process(taskRunner, processContext);

          CHECK(
            resourcesWereProcessed.notifications
            == std::vector<std::vector<ResourceId>>{{resource1->id()}});
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource1->state()));
          CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource2->state()));
        }
      }
    }

    SECTION("dropping resources")
    {
      auto mockDropCalls = std::array{false, false};
      auto sharedResources = std::array{
        std::make_shared<ResourceT>([&]() {
          return Result<MockResource>{MockResource{
            [](const auto&) {},
            [&](const auto&) { mockDropCalls[0] = true; },
          }};
        }),
        std::make_shared<ResourceT>([&]() {
          return Result<MockResource>{MockResource{
            [](const auto&) {},
            [&](const auto&) { mockDropCalls[1] = true; },
          }};
        }),
      };

      const auto resourceIds =
        sharedResources
        | std::views::transform([](const auto& resource) { return resource->id(); })
        | kdl::ranges::to<std::vector>();

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

      resourcesWereProcessed.reset();
      resourceManager.process(taskRunner, processContext);

      CHECK(
        resourcesWereProcessed.notifications
        == std::vector<std::vector<ResourceId>>{{resourceIds[0]}});
      CHECK(resourceManager.resources() == std::vector{sharedResources[1]});
      CHECK(mockDropCalls[0]);

      sharedResources[1].reset();
      CHECK(resourceManager.resources().size() == 1);

      resourcesWereProcessed.reset();
      resourceManager.process(taskRunner, processContext);

      CHECK(
        resourcesWereProcessed.notifications
        == std::vector<std::vector<ResourceId>>{{resourceIds[1]}});
      CHECK(resourceManager.resources().empty());
      CHECK(mockDropCalls[1]);
    }
  }
}

} // namespace tb::gl
