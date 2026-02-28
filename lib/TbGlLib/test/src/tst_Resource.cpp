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

#include "Result.h"
#include "gl/MockTaskRunner.h"
#include "gl/Resource.h"
#include "gl/TestGl.h"

#include "kd/reflection_impl.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::gl
{
struct MockResource
{
  void upload(Gl& gl) const { mockUpload(gl); }
  void drop(Gl& gl) const { mockDrop(gl); };

  std::function<void(Gl&)> mockUpload = [](auto&) {};
  std::function<void(Gl&)> mockDrop = [](auto&) {};

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
  auto testGl = TestGl{};
  auto mockTaskRunner = MockTaskRunner{};
  auto taskRunner = [&](auto task) { return mockTaskRunner.run(std::move(task)); };

  const auto processContext = ProcessContext{testGl, [](auto, auto) {}};

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
    auto mockUploadCall = false;
    auto mockDropCall = false;

    auto resource = ResourceT{[&]() {
      return Result<MockResource>{MockResource{
        [&](auto&) { mockUploadCall = true; },
        [&](auto&) { mockDropCall = true; },
      }};
    }};

    SECTION("ResourceUnloaded state")
    {
      setResourceState<ResourceUnloaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(!mockUploadCall);
      REQUIRE(!mockDropCall);

      CHECK(resource.get() == nullptr);
      CHECK(!resource.isDropped());
      CHECK(mockTaskRunner.tasks.empty());
      CHECK(!mockUploadCall);
      CHECK(!mockDropCall);

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.size() == 1);
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceUnloaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("dropSync")
      {
        resource.dropSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }
    }

    SECTION("ResourceLoading state")
    {
      setResourceState<ResourceLoading<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(!mockUploadCall);
      REQUIRE(!mockDropCall);

      SECTION("process")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          CHECK(!resource.process(taskRunner, processContext));
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          CHECK(resource.process(taskRunner, processContext));
          CHECK(resource.get() != nullptr);
          CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
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
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          resource.drop();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
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
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          resource.loadSync();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }
      }

      SECTION("uploadSync")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.uploadSync(testGl);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }

        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          resource.uploadSync(testGl);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }
      }

      SECTION("dropSync")
      {
        SECTION("TaskRunner has not resolved promise")
        {
          resource.dropSync(testGl);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.size() == 1);
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }


        SECTION("TaskRunner resolves promise")
        {
          mockTaskRunner.resolveNextPromise();
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceLoading<MockResource>>(resource.state()));
          CHECK(!resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);

          resource.dropSync(testGl);
          CHECK(resource.get() == nullptr);
          CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
          CHECK(resource.isDropped());
          CHECK(mockTaskRunner.tasks.empty());
          CHECK(!mockUploadCall);
          CHECK(!mockDropCall);
        }
      }
    }

    SECTION("ResourceLoaded state")
    {
      setResourceState<ResourceLoaded<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(!mockUploadCall);
      REQUIRE(!mockDropCall);

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceLoaded<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(testGl);
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("dropSync")
      {
        resource.dropSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }
    }

    SECTION("ResourceReady state")
    {
      setResourceState<ResourceReady<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall);
      REQUIRE(!mockDropCall);
      mockUploadCall = false;

      SECTION("process")
      {
        CHECK(!resource.process(taskRunner, processContext));
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(testGl);
        CHECK(resource.get() != nullptr);
        CHECK(std::holds_alternative<ResourceReady<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("dropSync")
      {
        resource.dropSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(mockDropCall);
      }
    }

    SECTION("ResourceDropping state")
    {
      setResourceState<ResourceDropping<MockResource>>(
        resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall);
      REQUIRE(!mockDropCall);
      mockUploadCall = false;

      SECTION("process")
      {
        CHECK(resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(mockDropCall);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropping<MockResource>>(resource.state()));
        CHECK(!resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("dropSync")
      {
        resource.dropSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(mockDropCall);
      }
    }

    SECTION("ResourceDropped state")
    {
      setResourceState<ResourceDropped>(resource, mockTaskRunner, processContext);
      REQUIRE(mockUploadCall);
      REQUIRE(mockDropCall);
      mockUploadCall = false;
      mockDropCall = false;

      SECTION("process")
      {
        CHECK(!resource.process(taskRunner, processContext));
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("drop")
      {
        resource.drop();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("loadSync")
      {
        resource.loadSync();
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("uploadSync")
      {
        resource.uploadSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
      }

      SECTION("dropSync")
      {
        resource.dropSync(testGl);
        CHECK(resource.get() == nullptr);
        CHECK(std::holds_alternative<ResourceDropped>(resource.state()));
        CHECK(resource.isDropped());
        CHECK(mockTaskRunner.tasks.empty());
        CHECK(!mockUploadCall);
        CHECK(!mockDropCall);
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

} // namespace tb::gl
