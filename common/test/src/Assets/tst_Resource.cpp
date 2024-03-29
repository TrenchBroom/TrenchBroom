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
#include "Error.h"
#include "Result.h"

#include "kdl/reflection_impl.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include "Catch2.h"

namespace TrenchBroom::Assets
{
struct MockResource
{
  void upload(const bool glContextAvailable) const { mockUpload(glContextAvailable); }
  void drop(const bool glContextAvailable) const { mockDrop(glContextAvailable); };

  std::function<void(bool)> mockUpload = [](auto) {};
  std::function<void(bool)> mockDrop = [](auto) {};

  kdl_reflect_inline_empty(MockResource);
};

using ResourceT = Resource<MockResource>;

template <typename State, typename MockTaskRunner>
void setResourceState(
  ResourceT& resource,
  MockTaskRunner& mockTaskRunner,
  const ProcessContext& processContext)
{
  static_assert(!std::is_same_v<State, ResourceFailed>);

  auto taskRunner = [&](auto task) { return mockTaskRunner.run(std::move(task)); };

  REQUIRE(std::holds_alternative<ResourceUnloaded<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  resource.process(taskRunner, processContext);
  REQUIRE(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  mockTaskRunner.resolveNextPromise();
  REQUIRE(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  resource.process(taskRunner, processContext);
  REQUIRE(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  resource.process(taskRunner, processContext);
  REQUIRE(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  resource.drop();
  REQUIRE(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
  if (std::holds_alternative<State>(resource.state()))
  {
    return;
  }

  resource.process(taskRunner, processContext);
  REQUIRE(std::holds_alternative<ResourceDropped>(resource.state()));
}

TEST_CASE("Resource")
{
  auto mockTaskRunner = MockTaskRunner{};
  auto taskRunner = [&](auto task) { return mockTaskRunner.run(std::move(task)); };

  const auto glContextAvailable = GENERATE(true, false);
  const auto processContext = ProcessContext{glContextAvailable};

  SECTION("Construction with loaded resource")
  {
    auto resource = ResourceT{MockResource{}};

    CHECK(resource.get() != nullptr);
    CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
    CHECK(!resource.isDropped());
    CHECK(mockTaskRunner.tasks.empty());
  }

  SECTION("Resource loading fails")
  {
    auto resource =
      ResourceT{[&]() { return Result<MockResource>{Error{"MockResource failed"}}; }};

    SECTION("async")
    {
      setResourceState<ResourceLoading<MockResource>>(
        resource, mockTaskRunner, processContext);
      mockTaskRunner.resolveNextPromise();

      CHECK(resource.process(taskRunner, processContext));
      CHECK(
        resource.state()
        == ResourceState<MockResource>{ResourceFailed{"MockResource failed"}});
    }

    SECTION("sync")
    {
      resource.loadSync();
      CHECK(
        resource.state()
        == ResourceState<MockResource>{ResourceFailed{"MockResource failed"}});
    }
  }

  SECTION("Resource loading succeeds")
  {
    auto mockUploadCall = std::optional<bool>{};
    auto mockDropCall = std::optional<bool>{};

    auto resource = ResourceT{[&]() {
      return Result<MockResource>{MockResource{
        [&](const auto i_glContextAvailable) { mockUploadCall = i_glContextAvailable; },
        [&](const auto i_glContextAvailable) { mockDropCall = i_glContextAvailable; },
      }};
    }};

    SECTION("ResourceUnloaded state")
    {
      setResourceState<ResourceUnloaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == std::nullopt);
      REQUIRE(mockDropCall == std::nullopt);

      CHECK(resource.get() == nullptr);
      CHECK(!resource.isDropped());
      CHECK(mockTaskRunner.tasks.empty());
      CHECK(mockUploadCall == std::nullopt);
      CHECK(mockDropCall == std::nullopt);

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.size() == 1);
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceUnloaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("dropSync")
      {
        resource.dropSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }
    }

    SECTION("ResourceLoading state")
    {
      setResourceState<ResourceLoading<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == std::nullopt);
      REQUIRE(mockDropCall == std::nullopt);

      SECTION("process")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          CHECK(!resource.process(taskRunner, processContext));
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          CHECK(resource.process(taskRunner, processContext));
          CHECK(resource.get() != nullptr);
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }
      }

      SECTION("drop")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.drop();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          resource.drop();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }
      }

      SECTION("loadSync")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.loadSync();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          resource.loadSync();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }
      }

      SECTION("uploadSync")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.uploadSync(glContextAvailable);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          resource.uploadSync(glContextAvailable);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }
      }

      SECTION("dropSync")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.dropSync(glContextAvailable);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }


        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);

          resource.dropSync(glContextAvailable);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(mockUploadCall == std::nullopt);
          CHECK(mockDropCall == std::nullopt);
        }
      }
    }

    SECTION("ResourceLoaded state")
    {
      setResourceState<ResourceLoaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == std::nullopt);
      REQUIRE(mockDropCall == std::nullopt);

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == glContextAvailable);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(glContextAvailable);
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == glContextAvailable);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("dropSync")
      {
        resource.dropSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }
    }

    SECTION("ResourceReady state")
    {
      setResourceState<ResourceReady<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == glContextAvailable);
      REQUIRE(mockDropCall == std::nullopt);
      mockUploadCall = std::nullopt;

      SECTION("process")
      {
        CHECK(!resource.process(taskRunner, processContext));
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(glContextAvailable);
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("dropSync")
      {
        resource.dropSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == glContextAvailable);
      }
    }

    SECTION("ResourceDropping state")
    {
      setResourceState<ResourceDropping<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == glContextAvailable);
      REQUIRE(mockDropCall == std::nullopt);
      mockUploadCall = std::nullopt;

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == glContextAvailable);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("dropSync")
      {
        resource.dropSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == glContextAvailable);
      }
    }

    SECTION("ResourceDropped state")
    {
      setResourceState<ResourceDropped>(resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall == glContextAvailable);
      REQUIRE(mockDropCall == glContextAvailable);
      mockUploadCall = std::nullopt;
      mockDropCall = std::nullopt;

      SECTION("process")
      {
        CHECK(!resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }

      SECTION("dropSync")
      {
        resource.dropSync(glContextAvailable);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall == std::nullopt);
        CHECK(mockDropCall == std::nullopt);
      }
    }
  }

  SECTION("needsProcessing")
  {
    SECTION("ResourceFailed state")
    {
      auto resource =
        ResourceT{[&]() { return Result<MockResource>{Error{"MockResource failed"}}; }};

      setResourceState<ResourceLoading<MockResource>>(
        resource, mockTaskRunner, processContext);
      mockTaskRunner.resolveNextPromise();

      resource.process(taskRunner, processContext);
      REQUIRE(
        resource.state()
        == ResourceState<MockResource>{ResourceFailed{"MockResource failed"}});
      CHECK(!resource.needsProcessing());
    }

    SECTION("ResourceUnloaded state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceUnloaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      CHECK(resource.needsProcessing());
    }

    SECTION("ResourceLoading state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceLoading<MockResource>>(
        resource, mockTaskRunner, processContext);
      CHECK(resource.needsProcessing());
    }

    SECTION("ResourceLoaded state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceLoaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      CHECK(resource.needsProcessing());
    }

    SECTION("ResourceReady state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceReady<MockResource>>(
        resource, mockTaskRunner, processContext);
      CHECK(!resource.needsProcessing());
    }

    SECTION("ResourceDropping state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceDropping<MockResource>>(
        resource, mockTaskRunner, processContext);
      CHECK(resource.needsProcessing());
    }

    SECTION("ResourceDropped state")
    {
      auto resource = ResourceT{[&]() { return Result<MockResource>{MockResource{}}; }};
      setResourceState<ResourceDropped>(resource, mockTaskRunner, processContext);
      CHECK(resource.needsProcessing());
    }
  }
}

} // namespace TrenchBroom::Assets
