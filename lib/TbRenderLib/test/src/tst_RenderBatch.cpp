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

#include "gl/FontManager.h"
#include "gl/OrthographicCamera.h"
#include "gl/ShaderManager.h"
#include "gl/TestGl.h"
#include "gl/VboManager.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/Renderable.h"

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{

// Records "prepared"/"rendered" into a shared log (rather than separate booleans) so
// each SECTION can assert inline that prepare() ran before render(), instead of that
// invariant being checked once, centrally, inside the class itself.

class RecordingRenderable : public Renderable
{
public:
  std::vector<std::string>* order;
  bool* destroyed;

  RecordingRenderable(std::vector<std::string>& orderLog, bool& destroyedFlag)
    : order{&orderLog}
    , destroyed{&destroyedFlag}
  {
  }

  ~RecordingRenderable() override { *destroyed = true; }

  void render(RenderContext&) override { order->emplace_back("rendered"); }
};

class RecordingDirectRenderable : public DirectRenderable
{
public:
  std::vector<std::string>* order;
  bool* destroyed;

  RecordingDirectRenderable(std::vector<std::string>& orderLog, bool& destroyedFlag)
    : order{&orderLog}
    , destroyed{&destroyedFlag}
  {
  }

  ~RecordingDirectRenderable() override { *destroyed = true; }

  void prepare(gl::Gl&, gl::VboManager&) override { order->emplace_back("prepared"); }
  void render(RenderContext&) override { order->emplace_back("rendered"); }
};

class RecordingIndexedRenderable : public IndexedRenderable
{
public:
  std::vector<std::string>* order;
  bool* destroyed;

  RecordingIndexedRenderable(std::vector<std::string>& orderLog, bool& destroyedFlag)
    : order{&orderLog}
    , destroyed{&destroyedFlag}
  {
  }

  ~RecordingIndexedRenderable() override { *destroyed = true; }

  void prepare(gl::Gl&, gl::VboManager&) override { order->emplace_back("prepared"); }
  void render(RenderContext&) override { order->emplace_back("rendered"); }
};

} // namespace

TEST_CASE("RenderBatch")
{
  auto testGl = gl::TestGl{};
  auto vboManager = gl::VboManager{};
  auto camera = gl::OrthographicCamera{};
  auto fontManager = gl::FontManager{[](const auto& path) { return path; }};
  auto shaderManager = gl::ShaderManager{[](const auto& path) { return path; }};
  auto renderContext =
    RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};

  SECTION("add")
  {
    SECTION("Renderable* renders it but does not take ownership")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.add(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"rendered"});
      }

      CHECK(!destroyed);
      delete renderable;
      CHECK(destroyed);
    }

    SECTION("DirectRenderable* prepares and renders it but does not take ownership")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingDirectRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.add(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"prepared", "rendered"});
      }

      CHECK(!destroyed);
      delete renderable;
      CHECK(destroyed);
    }

    SECTION(
      "IndexedRenderable* prepares and renders it through a wrapper, but does "
      "not take ownership of the original")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingIndexedRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.add(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"prepared", "rendered"});
      }

      CHECK(!destroyed);
      delete renderable;
      CHECK(destroyed);
    }
  }

  SECTION("addOneShot")
  {
    SECTION("Renderable* renders it and takes ownership")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.addOneShot(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"rendered"});
        CHECK(!destroyed);
      }

      CHECK(destroyed); // the batch deleted it on destruction
    }

    SECTION("DirectRenderable* prepares, renders and takes ownership")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingDirectRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.addOneShot(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"prepared", "rendered"});
        CHECK(!destroyed);
      }

      CHECK(destroyed);
    }

    SECTION("IndexedRenderable* prepares, renders and takes ownership of the original")
    {
      auto order = std::vector<std::string>{};
      auto destroyed = false;
      auto* renderable = new RecordingIndexedRenderable{order, destroyed};

      {
        auto batch = RenderBatch{vboManager};
        batch.addOneShot(renderable);
        batch.render(renderContext);
        CHECK(order == std::vector<std::string>{"prepared", "rendered"});
        CHECK(!destroyed);
      }

      CHECK(destroyed);
    }
  }

  SECTION("render renders every added renderable in the order they were added")
  {
    auto order = std::vector<int>{};

    class OrderRecordingRenderable : public Renderable
    {
    public:
      std::vector<int>& order;
      int id;

      OrderRecordingRenderable(std::vector<int>& orderVec, const int recordedId)
        : order{orderVec}
        , id{recordedId}
      {
      }

      void render(RenderContext&) override { order.push_back(id); }
    };

    auto first = OrderRecordingRenderable{order, 1};
    auto second = OrderRecordingRenderable{order, 2};

    auto batch = RenderBatch{vboManager};
    batch.add(&first);
    batch.add(&second);
    batch.render(renderContext);

    CHECK(order == std::vector<int>{1, 2});
  }
}

} // namespace tb::render
