/*
 Copyright (C) 2021 Kristian Duske

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

#include "PatchRenderer.h"

#include "Assets/Texture.h"
#include "Model/PatchNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TexturedIndexArrayMapBuilder.h"
#include "Renderer/TexturedIndexArrayRenderer.h"
#include "Renderer/VertexArray.h"

#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
namespace Renderer {
PatchRenderer::PatchRenderer()
  : m_grayscale(false)
  , m_tint(false)
  , m_alpha(1.0f) {}

void PatchRenderer::setDefaultColor(const Color& faceColor) {
  m_defaultColor = faceColor;
}

void PatchRenderer::setGrayscale(const bool grayscale) {
  m_grayscale = grayscale;
}

void PatchRenderer::setTint(const bool tint) {
  m_tint = tint;
}

void PatchRenderer::setTintColor(const Color& color) {
  m_tintColor = color;
}

void PatchRenderer::setTransparencyAlpha(const float alpha) {
  m_alpha = alpha;
}

void PatchRenderer::setShowEdges(const bool showEdges) {
  m_showEdges = showEdges;
}

void PatchRenderer::setEdgeColor(const RenderType type, const Color& edgeColor) {
  m_edgeColor[type] = edgeColor;
}

void PatchRenderer::setShowOccludedEdges(const bool showOccludedEdges) {
  m_showOccludedEdges = showOccludedEdges;
}

void PatchRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
  m_occludedEdgeColor = occludedEdgeColor;
}

void PatchRenderer::invalidate() {
  m_valid = false;
}

void PatchRenderer::clear() {
  m_patchNodes.clear();
  invalidate();
}

void PatchRenderer::addPatch(const Model::PatchNode* patchNode) {
  if (m_patchNodes.insert(patchNode).second) {
    invalidate();
  }
}

void PatchRenderer::removePatch(const Model::PatchNode* patchNode) {
  if (auto it = m_patchNodes.find(patchNode); it != std::end(m_patchNodes)) {
    m_patchNodes.erase(it);
    invalidate();
  }
}

void PatchRenderer::invalidatePatch(const Model::PatchNode*) {
  invalidate();
}

void PatchRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
  if (!m_valid) {
    validate();
  }

  if (renderContext.showFaces()) {
    renderBatch.add(this);
  }

  if (renderContext.showEdges()) {
    if (m_showOccludedEdges) {
      m_edgeRenderer.renderOnTop(renderBatch, m_occludedEdgeColor);
    }
    // FIXME: handle all colors
    m_edgeRenderer.render(renderBatch, m_edgeColor[RenderType::Default]);
  }
}

static TexturedIndexArrayRenderer buildMeshRenderer(
  const std::vector<const Model::PatchNode*>& patchNodes) {
  size_t vertexCount = 0u;
  auto indexArrayMapSize = TexturedIndexArrayMap::Size{};

  for (const auto* patchNode : patchNodes) {
    vertexCount += patchNode->grid().pointRowCount * patchNode->grid().pointColumnCount;

    const auto* texture = patchNode->patch().texture();
    const auto quadCount = patchNode->grid().quadRowCount() * patchNode->grid().quadColumnCount();
    indexArrayMapSize.inc(texture, PrimType::Triangles, 6u * quadCount);
  }

  using Vertex = GLVertexTypes::P3NT2::Vertex;
  auto vertices = std::vector<Vertex>{};
  vertices.reserve(vertexCount);

  auto indexArrayMapBuilder = TexturedIndexArrayMapBuilder{indexArrayMapSize};
  using Index = TexturedIndexArrayMapBuilder::Index;

  for (const auto* patchNode : patchNodes) {
    const auto vertexOffset = vertices.size();

    const auto& grid = patchNode->grid();
    auto gridVertices = kdl::vec_transform(grid.points, [](const auto& p) {
      return Vertex{vm::vec3f{p.position}, vm::vec3f{p.normal}, vm::vec2f{p.texCoords}};
    });
    vertices = kdl::vec_concat(std::move(vertices), std::move(gridVertices));

    const auto* texture = patchNode->patch().texture();

    const auto pointsPerRow = grid.pointColumnCount;
    for (size_t row = 0u; row < grid.quadRowCount(); ++row) {
      for (size_t col = 0u; col < grid.quadColumnCount(); ++col) {
        const auto i0 = vertexOffset + row * pointsPerRow + col;
        const auto i1 = vertexOffset + row * pointsPerRow + col + 1u;
        const auto i2 = vertexOffset + (row + 1u) * pointsPerRow + col + 1u;
        const auto i3 = vertexOffset + (row + 1u) * pointsPerRow + col;

        indexArrayMapBuilder.addTriangle(
          texture, static_cast<Index>(i0), static_cast<Index>(i1), static_cast<Index>(i2));
        indexArrayMapBuilder.addTriangle(
          texture, static_cast<Index>(i2), static_cast<Index>(i3), static_cast<Index>(i0));
      }
    }
  }

  auto vertexArray = VertexArray::move(std::move(vertices));
  auto indexArray = IndexArray::move(std::move(indexArrayMapBuilder.indices()));
  return TexturedIndexArrayRenderer{
    std::move(vertexArray), std::move(indexArray), std::move(indexArrayMapBuilder.ranges())};
}

static DirectEdgeRenderer buildEdgeRenderer(
  const std::vector<const Model::PatchNode*>& patchNodes) {
  size_t vertexCount = 0u;
  auto indexRangeMapSize = IndexRangeMap::Size{};

  for (const auto* patchNode : patchNodes) {
    vertexCount += (patchNode->grid().pointRowCount + patchNode->grid().pointColumnCount - 2u) * 2u;
    indexRangeMapSize.inc(PrimType::LineLoop, vertexCount);
  }

  auto indexRangeMapBuilder =
    IndexRangeMapBuilder<GLVertexTypes::P3>{vertexCount, indexRangeMapSize};

  for (const auto* patchNode : patchNodes) {
    const auto& grid = patchNode->grid();

    auto edgeLoopVertices = std::vector<GLVertexTypes::P3::Vertex>{};
    edgeLoopVertices.reserve((grid.pointRowCount + grid.pointColumnCount - 2u) * 2u);

    // walk around the patch to collect the edge vertices
    // for each side, collect the first vertex up to but not including the last vertex

    const auto t = 0u;
    const auto b = grid.pointRowCount - 1u;
    const auto l = 0u;
    const auto r = grid.pointColumnCount - 1u;

    size_t row = t;
    size_t col = l;

    while (col < r) {
      edgeLoopVertices.emplace_back(vm::vec3f{grid.point(row, col++).position});
    }
    assert(row == t && col == r);

    while (row < b) {
      edgeLoopVertices.emplace_back(vm::vec3f{grid.point(row++, col).position});
    }
    assert(row == b && col == r);

    while (col > l) {
      edgeLoopVertices.emplace_back(vm::vec3f{grid.point(row, col--).position});
    }
    assert(row == b && col == l);

    while (row > t) {
      edgeLoopVertices.emplace_back(vm::vec3f{grid.point(row--, col).position});
    }
    assert(row == t && col == l);

    indexRangeMapBuilder.addLineLoop(std::move(edgeLoopVertices));
  }

  auto vertexArray = VertexArray::move(std::move(indexRangeMapBuilder.vertices()));
  auto indexRangeMap = std::move(indexRangeMapBuilder.indices());
  return DirectEdgeRenderer{std::move(vertexArray), std::move(indexRangeMap)};
}

void PatchRenderer::validate() {
  if (!m_valid) {
    m_patchMeshRenderer = buildMeshRenderer(m_patchNodes.get_data());
    m_edgeRenderer = buildEdgeRenderer(m_patchNodes.get_data());

    m_valid = true;
  }
}

void PatchRenderer::prepareVerticesAndIndices(VboManager& vboManager) {
  m_patchMeshRenderer.prepare(vboManager);
}

namespace {
struct RenderFunc : public TextureRenderFunc {
  ActiveShader& shader;
  bool applyTexture;
  const Color& defaultColor;

  RenderFunc(ActiveShader& i_shader, const bool i_applyTexture, const Color& i_defaultColor)
    : shader(i_shader)
    , applyTexture(i_applyTexture)
    , defaultColor(i_defaultColor) {}

  void before(const Assets::Texture* texture) override {
    shader.set("GridColor", gridColorForTexture(texture));
    if (texture != nullptr) {
      texture->activate();
      shader.set("ApplyTexture", applyTexture);
      shader.set("Color", texture->averageColor());
    } else {
      shader.set("ApplyTexture", false);
      shader.set("Color", defaultColor);
    }
  }

  void after(const Assets::Texture* texture) override {
    if (texture != nullptr) {
      texture->deactivate();
    }
  }
};
} // namespace

void PatchRenderer::doRender(RenderContext& context) {
  return; // FIXME:
  ShaderManager& shaderManager = context.shaderManager();
  ActiveShader shader(shaderManager, Shaders::FaceShader);
  PreferenceManager& prefs = PreferenceManager::instance();

  const bool applyTexture = context.showTextures();
  const bool shadeFaces = context.shadeFaces();
  const bool showFog = context.showFog();

  glAssert(glEnable(GL_TEXTURE_2D));
  glAssert(glActiveTexture(GL_TEXTURE0));
  shader.set("Brightness", prefs.get(Preferences::Brightness));
  shader.set("RenderGrid", context.showGrid());
  shader.set("GridSize", static_cast<float>(context.gridSize()));
  shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
  shader.set("ApplyTexture", applyTexture);
  shader.set("Texture", 0);
  shader.set("ApplyTinting", m_tint);
  if (m_tint) {
    shader.set("TintColor", m_tintColor);
  }
  shader.set("GrayScale", m_grayscale);
  shader.set("CameraPosition", context.camera().position());
  shader.set("ShadeFaces", shadeFaces);
  shader.set("ShowFog", showFog);
  shader.set("Alpha", 1.0);
  shader.set("EnableMasked", false);
  shader.set("ShowSoftMapBounds", !context.softMapBounds().is_empty());
  shader.set("SoftMapBoundsMin", context.softMapBounds().min);
  shader.set("SoftMapBoundsMax", context.softMapBounds().max);
  shader.set(
    "SoftMapBoundsColor", vm::vec4f(
                            prefs.get(Preferences::SoftMapBoundsColor).r(),
                            prefs.get(Preferences::SoftMapBoundsColor).g(),
                            prefs.get(Preferences::SoftMapBoundsColor).b(), 0.1f));

  RenderFunc func(shader, applyTexture, m_defaultColor);
  /*
  if (m_alpha < 1.0f) {
      glAssert(glDepthMask(GL_FALSE));
  }
  */

  m_patchMeshRenderer.render(func);

  /*
  if (m_alpha < 1.0f) {
      glAssert(glDepthMask(GL_TRUE));
  }
  */
}
} // namespace Renderer
} // namespace TrenchBroom
