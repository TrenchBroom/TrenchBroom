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
#include "render/RenderContext.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
auto makeFontManager()
{
  return gl::FontManager{[](const auto& path) { return path; }};
}

auto makeShaderManager()
{
  return gl::ShaderManager{[](const auto& path) { return path; }};
}
} // namespace

TEST_CASE("RenderContext")
{
  auto testGl = gl::TestGl{};
  auto camera = gl::OrthographicCamera{};
  auto fontManager = makeFontManager();
  auto shaderManager = makeShaderManager();

  SECTION("constructor exposes the given references")
  {
    auto context =
      RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};
    CHECK(&context.gl() == &testGl);
    CHECK(&context.camera() == &camera);
    CHECK(&context.fontManager() == &fontManager);
    CHECK(&context.shaderManager() == &shaderManager);
  }

  SECTION("render2D and render3D")
  {
    auto context2D =
      RenderContext{testGl, RenderMode::Render2D, camera, fontManager, shaderManager};
    CHECK(context2D.render2D());
    CHECK(!context2D.render3D());

    auto context3D =
      RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};
    CHECK(!context3D.render2D());
    CHECK(context3D.render3D());
  }

  SECTION("showFaces and setShowFaces")
  {
    auto context2D =
      RenderContext{testGl, RenderMode::Render2D, camera, fontManager, shaderManager};
    CHECK(!context2D.showFaces());
    context2D.setShowFaces(true);
    CHECK(!context2D.showFaces()); // showFaces is only true in 3D render mode

    auto context3D =
      RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};
    CHECK(context3D.showFaces());
    context3D.setShowFaces(false);
    CHECK(!context3D.showFaces());
  }

  SECTION("showEdges and setShowEdges")
  {
    auto context2D =
      RenderContext{testGl, RenderMode::Render2D, camera, fontManager, shaderManager};
    context2D.setShowEdges(false);
    CHECK(context2D.showEdges()); // showEdges is always true in 2D render mode

    auto context3D =
      RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};
    CHECK(context3D.showEdges());
    context3D.setShowEdges(false);
    CHECK(!context3D.showEdges());
  }

  auto context =
    RenderContext{testGl, RenderMode::Render3D, camera, fontManager, shaderManager};

  SECTION("minFilterMode, magFilterMode and setFilterMode")
  {
    context.setFilterMode(GL_NEAREST, GL_LINEAR);
    CHECK(context.minFilterMode() == GL_NEAREST);
    CHECK(context.magFilterMode() == GL_LINEAR);
  }

  SECTION("showMaterials and setShowMaterials")
  {
    context.setShowMaterials(false);
    CHECK(!context.showMaterials());
  }

  SECTION("shadeFaces and setShadeFaces")
  {
    context.setShadeFaces(false);
    CHECK(!context.shadeFaces());
  }

  SECTION("showPointEntities and setShowPointEntities")
  {
    context.setShowPointEntities(false);
    CHECK(!context.showPointEntities());
  }

  SECTION("showPointEntityModels and setShowPointEntityModels")
  {
    context.setShowPointEntityModels(false);
    CHECK(!context.showPointEntityModels());
  }

  SECTION("showEntityClassnames and setShowEntityClassnames")
  {
    context.setShowEntityClassnames(false);
    CHECK(!context.showEntityClassnames());
  }

  SECTION("showGroupBounds and setShowGroupBounds")
  {
    context.setShowGroupBounds(false);
    CHECK(!context.showGroupBounds());
  }

  SECTION("showBrushEntityBounds and setShowBrushEntityBounds")
  {
    context.setShowBrushEntityBounds(false);
    CHECK(!context.showBrushEntityBounds());
  }

  SECTION("showPointEntityBounds and setShowPointEntityBounds")
  {
    context.setShowPointEntityBounds(false);
    CHECK(!context.showPointEntityBounds());
  }

  SECTION("showFog and setShowFog")
  {
    context.setShowFog(true);
    CHECK(context.showFog());
  }

  SECTION("showGrid and setShowGrid")
  {
    context.setShowGrid(false);
    CHECK(!context.showGrid());
  }

  SECTION("gridSize and setGridSize")
  {
    context.setGridSize(16.0);
    CHECK(context.gridSize() == 16.0);
  }

  SECTION("dpiScale and setDpiScale")
  {
    context.setDpiScale(2.0f);
    CHECK(context.dpiScale() == 2.0f);
  }

  SECTION("softMapBounds and setSoftMapBounds")
  {
    const auto bounds = vm::bbox3f{{-1, -1, -1}, {1, 1, 1}};
    context.setSoftMapBounds(bounds);
    CHECK(context.softMapBounds() == bounds);
  }

  SECTION("hideSelection and setHideSelection")
  {
    CHECK(!context.hideSelection());
    context.setHideSelection();
    CHECK(context.hideSelection()); // can only be set, never cleared
  }

  SECTION("tintSelection and clearTintSelection")
  {
    context.clearTintSelection();
    CHECK(!context.tintSelection()); // can only be cleared, never set
  }

  SECTION("showSelectionGuide and its setters")
  {
    SECTION("starts out hidden")
    {
      CHECK(!context.showSelectionGuide());
    }

    SECTION("setShowSelectionGuide shows the guide from the hidden state")
    {
      context.setHideSelectionGuide();
      context.setShowSelectionGuide();
      CHECK(context.showSelectionGuide());
    }

    SECTION("setHideSelectionGuide hides the guide from the shown state")
    {
      context.setShowSelectionGuide();
      context.setHideSelectionGuide();
      CHECK(!context.showSelectionGuide());
    }

    SECTION("setForceShowSelectionGuide always shows the guide")
    {
      context.setHideSelectionGuide();
      context.setForceShowSelectionGuide();
      CHECK(context.showSelectionGuide());
    }

    SECTION("setForceHideSelectionGuide hides the guide even when it was shown")
    {
      context.setShowSelectionGuide();
      context.setForceHideSelectionGuide();
      CHECK(!context.showSelectionGuide());
    }

    SECTION("setForceHideSelectionGuide cannot override a forced show")
    {
      context.setForceShowSelectionGuide();
      context.setForceHideSelectionGuide();
      CHECK(context.showSelectionGuide());
    }

    SECTION("a plain hide cannot override a forced show")
    {
      context.setForceShowSelectionGuide();
      context.setHideSelectionGuide();
      CHECK(context.showSelectionGuide());
    }

    SECTION("setForceShowSelectionGuide overrides an earlier forced hide")
    {
      // ForceShow is unconditional, unlike ForceHide (which backs off when already
      // ForceShow), so it always wins regardless of what preceded it
      context.setForceHideSelectionGuide();
      context.setForceShowSelectionGuide();
      CHECK(context.showSelectionGuide());
    }
  }
}

} // namespace tb::render
