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

#include "TestPreferenceStore.h"
#include "base/PreferenceManager.h"
#include "gl/FontManager.h"
#include "gl/Material.h"
#include "gl/MockGl.h"
#include "gl/OrthographicCamera.h"
#include "gl/ShaderManager.h"
#include "gl/Shaders.h"
#include "gl/TestUtils.h"
#include "gl/Texture.h"
#include "gl/VboManager.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/MapFormat.h"
#include "render/BrushRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"

#include "kd/filesystem_utils.h"
#include "kd/result.h"

#include "vm/bbox.h"

#include <fstream>
#include <optional>
#include <unordered_map>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{

// creates the global PreferenceManager instance (FaceRenderer::render() reads
// Preferences::Brightness etc.) and destroys it again when it goes out of scope, so no
// instance leaks into other test cases
struct PreferenceManagerInstance
{
  PreferenceManagerInstance()
  {
    PreferenceManager::createInstance(std::make_unique<TestPreferenceStore>(), true);
  }

  ~PreferenceManagerInstance() { PreferenceManager::destroyInstance(); }
};

mdl::BrushNode makeCubeBrushNode(
  gl::Material& material, const vm::vec3d& center = vm::vec3d{0, 0, 0})
{
  const auto worldBounds = vm::bbox3d{4096.0};
  auto builder = mdl::BrushBuilder{mdl::MapFormat::Standard, worldBounds};
  const auto bounds = vm::bbox3d{
    center - vm::vec3d{32.0, 32.0, 32.0}, center + vm::vec3d{32.0, 32.0, 32.0}};
  auto brush = builder.createCuboid(bounds, "material") | kdl::value();
  for (auto& face : brush.faces())
  {
    face.setMaterial(&material);
  }
  return mdl::BrushNode{std::move(brush)};
}

} // namespace

TEST_CASE("BrushRenderer rendering")
{
  auto preferenceManager = PreferenceManagerInstance{};

  auto tmpFile = kdl::tmp_file{};
  {
    auto ofs = std::ofstream{tmpFile.path()};
    ofs << "void main() {}\n";
  }

  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);
  gl::installShaderCompileSupport(gl);

  // Every uniform name gets a stable, distinct location the first time it's looked up,
  // so uniform value callbacks below can identify which uniform they belong to.
  auto locationToName = std::unordered_map<GLint, std::string>{};
  gl.onGetUniformLocation = [&, nextLocation = GLint{1}](
                              GLuint, const GLchar* name) mutable -> GLint {
    for (const auto& [location, existingName] : locationToName)
    {
      if (existingName == name)
      {
        return location;
      }
    }
    const auto location = nextLocation++;
    locationToName[location] = name;
    return location;
  };

  auto capturedAlphas = std::vector<GLfloat>{};
  // tracks the most recently set AlphaFuncThreshold, so onDrawElements below can record
  // which material's item was active for each draw call, to verify sorted draw order
  auto currentAlphaFuncThreshold = std::optional<GLfloat>{};
  gl.onUniform1f = [&](const GLint location, const GLfloat value) {
    const auto& name = locationToName.at(location);
    if (name == "Alpha")
    {
      capturedAlphas.push_back(value);
    }
    else if (name == "AlphaFuncThreshold")
    {
      currentAlphaFuncThreshold = value;
    }
  };
  gl.onUniform1i = [](GLint, GLint) {};
  gl.onUniform3f = [](GLint, GLfloat, GLfloat, GLfloat) {};
  gl.onUniform4f = [](GLint, GLfloat, GLfloat, GLfloat, GLfloat) {};

  gl.onUseProgram = [](GLuint) {};
  gl.onGetIntegerv = [](const GLenum pname, GLint* params) {
    if (pname == GL_CURRENT_PROGRAM)
    {
      *params = GLint{1};
    }
  };

  gl.onEnable = [](GLenum) {};
  gl.onDisable = [](GLenum) {};
  gl.onActiveTexture = [](GLenum) {};

  gl.onMatrixMode = [](GLenum) {};
  gl.onLoadMatrixd = [](const GLdouble*) {};
  gl.onLoadMatrixf = [](const GLfloat*) {};

  gl.onVertexPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
  gl.onNormalPointer = [](GLenum, GLsizei, const GLvoid*) {};
  gl.onTexCoordPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
  gl.onEnableClientState = [](GLenum) {};
  gl.onDisableClientState = [](GLenum) {};
  gl.onClientActiveTexture = [](GLenum) {};

  auto depthMaskCalls = std::vector<GLboolean>{};
  gl.onDepthMask = [&](const GLboolean flag) { depthMaskCalls.push_back(flag); };

  auto drawCallCount = 0;
  auto drawnThresholds = std::vector<GLfloat>{};
  gl.onDrawElements = [&](GLenum, GLsizei, GLenum, const void*) {
    ++drawCallCount;
    if (currentAlphaFuncThreshold)
    {
      drawnThresholds.push_back(*currentAlphaFuncThreshold);
    }
  };

  auto vboManager = gl::VboManager{};
  auto shaderManager =
    gl::ShaderManager{[&](const std::filesystem::path&) { return tmpFile.path(); }};
  REQUIRE(shaderManager.loadProgram(gl, gl::Shaders::FaceShader).is_success());

  auto camera = gl::OrthographicCamera{};
  auto fontManager = gl::FontManager{[](const auto& path) { return path; }};
  auto renderContext =
    RenderContext{gl, RenderMode::Render3D, camera, fontManager, shaderManager};
  renderContext.setShowEdges(false);

  auto opaqueMaterial = gl::Material{"opaque", createTextureResource(gl::Texture{4, 4})};

  auto blendMaterial = gl::Material{"blend", createTextureResource(gl::Texture{4, 4})};
  blendMaterial.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  SECTION(
    "a material with real blend factors is drawn in the transparent pass, with depth "
    "writes disabled, even when TransparencyAlpha is 1.0")
  {
    auto renderer = BrushRenderer{};
    auto brushNode = makeCubeBrushNode(blendMaterial);
    renderer.addBrush(brushNode);

    auto batch = RenderBatch{vboManager};
    renderer.render(renderContext, batch);
    batch.render(renderContext);

    CHECK(depthMaskCalls == std::vector<GLboolean>{GL_FALSE, GL_TRUE});
    CHECK(drawCallCount == 1);
  }

  SECTION("a material without real blend factors stays in the opaque pass")
  {
    auto renderer = BrushRenderer{};
    auto brushNode = makeCubeBrushNode(opaqueMaterial);
    renderer.addBrush(brushNode);

    auto batch = RenderBatch{vboManager};
    renderer.render(renderContext, batch);
    batch.render(renderContext);

    CHECK(depthMaskCalls.empty());
    CHECK(drawCallCount == 1);
  }

  SECTION(
    "a material with real blend factors renders with its own alpha, not the "
    "whole-batch X-ray/hidden-brush fade")
  {
    auto renderer = BrushRenderer{};
    renderer.setTransparencyAlpha(0.4f);

    auto brushNode = makeCubeBrushNode(blendMaterial);
    renderer.addBrush(brushNode);

    auto batch = RenderBatch{vboManager};
    renderer.render(renderContext, batch);
    batch.render(renderContext);

    REQUIRE(!capturedAlphas.empty());
    CHECK(capturedAlphas.back() == 1.0f);
  }

  SECTION("transparent brushes are drawn back-to-front by distance to the camera")
  {
    auto nearMaterial = gl::Material{"near", createTextureResource(gl::Texture{4, 4})};
    nearMaterial.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    nearMaterial.setAlphaFunc(gl::MaterialAlphaFunc::Compare::GreaterEqual, 0.1f);

    auto midMaterial = gl::Material{"mid", createTextureResource(gl::Texture{4, 4})};
    midMaterial.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    midMaterial.setAlphaFunc(gl::MaterialAlphaFunc::Compare::GreaterEqual, 0.2f);

    auto farMaterial = gl::Material{"far", createTextureResource(gl::Texture{4, 4})};
    farMaterial.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    farMaterial.setAlphaFunc(gl::MaterialAlphaFunc::Compare::GreaterEqual, 0.3f);

    // the default camera is at the origin; brushes are added in an order that does not
    // match distance to it, to confirm the renderer is actually sorting rather than
    // just preserving insertion or material-map order
    auto renderer = BrushRenderer{};
    auto midBrush = makeCubeBrushNode(midMaterial, vm::vec3d{200.0, 0.0, 0.0});
    auto farBrush = makeCubeBrushNode(farMaterial, vm::vec3d{300.0, 0.0, 0.0});
    auto nearBrush = makeCubeBrushNode(nearMaterial, vm::vec3d{100.0, 0.0, 0.0});
    renderer.addBrush(midBrush);
    renderer.addBrush(farBrush);
    renderer.addBrush(nearBrush);

    auto batch = RenderBatch{vboManager};
    renderer.render(renderContext, batch);
    batch.render(renderContext);

    CHECK(drawnThresholds == std::vector<GLfloat>{0.3f, 0.2f, 0.1f});
  }

  SECTION(
    "transparent brushes sharing a material are no longer batched into one draw call")
  {
    auto renderer = BrushRenderer{};
    auto brush1 = makeCubeBrushNode(blendMaterial, vm::vec3d{100.0, 0.0, 0.0});
    auto brush2 = makeCubeBrushNode(blendMaterial, vm::vec3d{200.0, 0.0, 0.0});
    renderer.addBrush(brush1);
    renderer.addBrush(brush2);

    auto batch = RenderBatch{vboManager};
    renderer.render(renderContext, batch);
    batch.render(renderContext);

    CHECK(drawCallCount == 2);
  }

  vboManager.destroyPendingVbos(gl);
}

TEST_CASE("BrushRenderer")
{
  SECTION("triIndicesCountForPolygon")
  {
    SECTION("a triangle needs 3 indices (1 triangle)")
    {
      CHECK(triIndicesCountForPolygon(3) == 3u);
    }

    SECTION("a quad needs 6 indices (2 triangles)")
    {
      CHECK(triIndicesCountForPolygon(4) == 6u);
    }

    SECTION("a pentagon needs 9 indices (3 triangles)")
    {
      CHECK(triIndicesCountForPolygon(5) == 9u);
    }
  }

  SECTION("addTriIndicesForPolygon")
  {
    SECTION("a triangle produces a single triangle referencing all three vertices")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(3));
      addTriIndicesForPolygon(dest.data(), 0, 3);
      CHECK(dest == std::vector<GLuint>{0, 1, 2});
    }

    SECTION("a quad produces a fan of two triangles sharing the base vertex")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(4));
      addTriIndicesForPolygon(dest.data(), 0, 4);
      CHECK(dest == std::vector<GLuint>{0, 1, 2, 0, 2, 3});
    }

    SECTION("a pentagon produces a fan of three triangles sharing the base vertex")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(5));
      addTriIndicesForPolygon(dest.data(), 0, 5);
      CHECK(dest == std::vector<GLuint>{0, 1, 2, 0, 2, 3, 0, 3, 4});
    }

    SECTION("indices are offset by the given base index")
    {
      auto dest = std::vector<GLuint>(triIndicesCountForPolygon(4));
      addTriIndicesForPolygon(dest.data(), 10, 4);
      CHECK(dest == std::vector<GLuint>{10, 11, 12, 10, 12, 13});
    }
  }
}

} // namespace tb::render
