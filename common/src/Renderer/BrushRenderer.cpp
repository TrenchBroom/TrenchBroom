/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "BrushRenderer.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Polyhedron.h"
#include "Model/TagAttribute.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/RenderContext.h"

#include <cassert>
#include <cstring>
#include <vector>

namespace TrenchBroom {
namespace Renderer {

/**
 * Rendering overview:
 * There are 2 things to render: brush faces (filled/textured polygons) and brush edges.
 *
 * For faces, we need to write a copy of each vertex for each face it's used on, because
 * of the texture coordinates/normal and face selection state being unique per face.
 *
 * For edges, it's a it more complicated because we only want to draw each edge once, even
 * though it's shared between 2 faces.
 */

// BrushRenderer

BrushRenderer::BrushRenderer()
  : m_showEdges(false)
  , m_grayscale(false)
  , m_tint(false)
  , m_showOccludedEdges(false)
  , m_forceTransparent(false)
  , m_transparencyAlpha(1.0f)
  , m_editorContext(nullptr) {
  clear();
}

BrushRenderer::BrushRenderer(const Model::EditorContext& editorContext)
  : BrushRenderer::BrushRenderer() {
  m_editorContext = &editorContext;
}

void BrushRenderer::invalidate() {
  for (auto& brush : m_allBrushes) {
    // this will also invalidate already invalid brushes, which
    // is unnecessary
    removeBrushFromVbo(brush);
  }
  m_invalidBrushes = m_allBrushes;

  assert(m_brushInfo.empty());
  assert(m_transparentFaces->empty());
  assert(m_opaqueFaces->empty());
}

void BrushRenderer::invalidateBrush(const Model::BrushNode* brush) {
  // skip brushes that are not in the renderer
  if (m_allBrushes.find(brush) == std::end(m_allBrushes)) {
    assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
    assert(m_invalidBrushes.find(brush) == std::end(m_invalidBrushes));
    return;
  }
  // if it's not in the invalid set, put it in
  if (m_invalidBrushes.insert(brush).second) {
    removeBrushFromVbo(brush);
  }
}

bool BrushRenderer::valid() const {
  return m_invalidBrushes.empty();
}

void BrushRenderer::clear() {
  m_brushInfo.clear();
  m_allBrushes.clear();
  m_invalidBrushes.clear();

  m_edgeVertices = std::make_shared<BrushEdgeVertexArray>();

  m_vertexArray = std::make_shared<BrushVertexArray>();
  m_transparentFaces = std::make_shared<TextureToBrushIndicesMap>();
  m_opaqueFaces = std::make_shared<TextureToBrushIndicesMap>();

  m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
  m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
  m_edgeRenderer = DirectBrushEdgeRenderer(m_edgeVertices);
}

void BrushRenderer::setFaceColor(const Color& faceColor) {
  m_faceColor = faceColor;
}

void BrushRenderer::setShowEdges(const bool showEdges) {
  m_showEdges = showEdges;
}

void BrushRenderer::setEdgeColor(const RenderType type, const Color& edgeColor) {
  m_edgeColor[type] = edgeColor;
}

void BrushRenderer::setGrayscale(const bool grayscale) {
  m_grayscale = grayscale;
}

void BrushRenderer::setTint(const bool tint) {
  m_tint = tint;
}

void BrushRenderer::setTintColor(const Color& tintColor) {
  m_tintColor = tintColor;
}

void BrushRenderer::setShowOccludedEdges(const bool showOccludedEdges) {
  m_showOccludedEdges = showOccludedEdges;
}

void BrushRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
  m_occludedEdgeColor = occludedEdgeColor;
}

void BrushRenderer::setForceTransparent(const bool transparent) {
  if (transparent != m_forceTransparent) {
    m_forceTransparent = transparent;
    invalidate();
  }
}

void BrushRenderer::setTransparencyAlpha(const float transparencyAlpha) {
  if (transparencyAlpha != m_transparencyAlpha) {
    m_transparencyAlpha = transparencyAlpha;
    invalidate();
  }
}

void BrushRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
  renderOpaque(renderContext, renderBatch);
  renderTransparent(renderContext, renderBatch);
}

void BrushRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
  if (!m_allBrushes.empty()) {
    if (!valid()) {
      validate();
    }
    if (renderContext.showFaces()) {
      renderOpaqueFaces(renderBatch);
    }
    if (renderContext.showEdges() || m_showEdges) {
      renderEdges(renderBatch);
    }
  }
}

void BrushRenderer::renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
  if (!m_allBrushes.empty()) {
    if (!valid()) {
      validate();
    }
    if (renderContext.showFaces()) {
      renderTransparentFaces(renderBatch);
    }
  }
}

void BrushRenderer::renderOpaqueFaces(RenderBatch& renderBatch) {
  m_opaqueFaceRenderer.setGrayscale(m_grayscale);
  m_opaqueFaceRenderer.setTint(m_tint);
  m_opaqueFaceRenderer.setTintColor(m_tintColor);
  m_opaqueFaceRenderer.render(renderBatch);
}

void BrushRenderer::renderTransparentFaces(RenderBatch& renderBatch) {
  m_transparentFaceRenderer.setGrayscale(m_grayscale);
  m_transparentFaceRenderer.setTint(m_tint);
  m_transparentFaceRenderer.setTintColor(m_tintColor);
  m_transparentFaceRenderer.setAlpha(m_transparencyAlpha);
  m_transparentFaceRenderer.render(renderBatch);
}

void BrushRenderer::renderEdges(RenderBatch& renderBatch) {
  //            if (m_showOccludedEdges) {

  m_edgeRenderer.render(renderBatch); //, m_edgeColor); -- disable uniform color

  // we tell the shader that we're rendering on top (no depth test/write)
  // and it can discard vertices
  m_edgeRenderer.renderOnTop(renderBatch); //, m_occludedEdgeColor);
}

void BrushRenderer::validate() {
  assert(!valid());

  for (auto brush : m_invalidBrushes) {
    validateBrush(brush);
  }
  m_invalidBrushes.clear();
  assert(valid());

  m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
  m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
  m_edgeRenderer = DirectBrushEdgeRenderer(m_edgeVertices);
}

static size_t triIndicesCountForPolygon(const size_t vertexCount) {
  assert(vertexCount >= 3);
  const size_t indexCount = 3 * (vertexCount - 2);
  return indexCount;
}

static void addTriIndicesForPolygon(
  GLuint* dest, const GLuint baseIndex, const size_t vertexCount) {
  assert(vertexCount >= 3);
  for (size_t i = 0; i < vertexCount - 2; ++i) {
    *(dest++) = baseIndex;
    *(dest++) = baseIndex + static_cast<GLuint>(i + 1);
    *(dest++) = baseIndex + static_cast<GLuint>(i + 2);
  }
}

bool BrushRenderer::shouldDrawFaceInTransparentPass(
  const Model::BrushNode* brush, const Model::BrushFace& face) const {
  if (m_transparencyAlpha >= 1.0f) {
    // In this case, draw everything in the opaque pass
    // see: https://github.com/kduske/TrenchBroom/issues/2848
    return false;
  }

  if (m_forceTransparent) {
    return true;
  }
  if (brush->hasAttribute(Model::TagAttributes::Transparency)) {
    return true;
  }
  if (face.hasAttribute(Model::TagAttributes::Transparency)) {
    return true;
  }
  return false;
}

/**
 * This exists so we can evaluate the rendering style once and pass it around
 * in an integer.
 */
BrushRenderFlags::Type BrushRenderer::brushRenderFlags(const Model::BrushNode* brush) const {
  BrushRenderFlags::Type result = 0u;

  if (m_editorContext != nullptr && !m_editorContext->visible(brush)) {
    result |= BrushRenderFlags::Hidden;
  }

  if (brush->transitivelySelected()) {
    result |= BrushRenderFlags::Selected;
  }

  if (brush->locked()) {
    result |= BrushRenderFlags::Locked;
  }

  return result;
}

BrushRenderFlags::Type BrushRenderer::edgeRenderFlags(
  BrushRenderFlags::Type brushFlags, const Model::BrushFace& face1,
  const Model::BrushFace& face2) const {
  BrushRenderFlags::Type flags = brushFlags;

  if (face1.selected() || face2.selected()) {
    flags |= BrushRenderFlags::Selected;
  }

  return flags;
}

BrushRenderFlags::Type BrushRenderer::faceRenderFlags(
  const Model::BrushNode* brush, BrushRenderFlags::Type brushFlags,
  const Model::BrushFace& face) const {
  BrushRenderFlags::Type flags = brushFlags;

  if (m_editorContext != nullptr && !m_editorContext->visible(brush, face)) {
    flags |= BrushRenderFlags::Hidden;
  }
  if (face.selected()) {
    flags |= BrushRenderFlags::Selected;
  }

  return flags;
}

vm::vec4f BrushRenderer::edgeColor(
  const BrushRenderFlags::Type brushFlags, const Model::BrushFace& face1,
  const Model::BrushFace& face2) const {
  const BrushRenderFlags::Type edgeFlags = edgeRenderFlags(brushFlags, face1, face2);

  // FIXME: temporary color
  if (edgeFlags & BrushRenderFlags::Locked) {
    return vm::vec4f(0, 0, 1, 1);
  }
  if (edgeFlags & BrushRenderFlags::Selected) {
    return vm::vec4f(1, 0, 0, 1);
  }
  return vm::vec4f(1, 1, 1, 1);
}

struct CachedFace {
  const Assets::Texture* texture;
  const Model::BrushFace* face;
  size_t vertexCount;
  /**
   * Relative to the start of the VBO
   */
  size_t indexOfFirstVertex;

  CachedFace(const Model::BrushFace* i_face, size_t i_indexOfFirstVertex)
    : texture(i_face->texture())
    , face(i_face)
    , vertexCount(i_face->vertexCount())
    , indexOfFirstVertex(i_indexOfFirstVertex) {}
};

void BrushRenderer::validateBrush(const Model::BrushNode* brush) {
  assert(m_allBrushes.find(brush) != std::end(m_allBrushes));
  assert(m_invalidBrushes.find(brush) != std::end(m_invalidBrushes));
  assert(m_brushInfo.find(brush) == std::end(m_brushInfo));

  // At this point, brush is not in the VBO's and will not be rendered.

  const BrushRenderFlags::Type brushFlags = brushRenderFlags(brush);
  if (brushFlags & BrushRenderFlags::Hidden) {
    return;
  }

  // The remainder of this function will fill in all of the fields of info
  BrushInfo& info = m_brushInfo[brush];
  const Model::Brush& brushValue = brush->brush();

  // insert edge vertices into VBO
  {
    const size_t edgeVertCount = 2 * brushValue.edges().size();
    auto [vertKey, vertDest] = m_edgeVertices->getPointerToInsertVerticesAt(edgeVertCount);
    info.edgeVerticesKey = vertKey;

    size_t i = 0;
    for (const Model::BrushEdge* currentEdge : brushValue.edges()) {
      const auto faceIndex1 = currentEdge->firstFace()->payload();
      const auto faceIndex2 = currentEdge->secondFace()->payload();
      assert(faceIndex1 && faceIndex2);

      const auto& face1 = brushValue.face(*faceIndex1);
      const auto& face2 = brushValue.face(*faceIndex2);

      const vm::vec3f pos1 = vm::vec3f(currentEdge->firstVertex()->position());
      const vm::vec3f pos2 = vm::vec3f(currentEdge->secondVertex()->position());
      const vm::vec4f color = edgeColor(brushFlags, face1, face2);
      const auto flags = edgeRenderFlags(brushFlags, face1, face2);

      vertDest[i++] = BrushEdgeVertex(
        pos1, vm::vec<uint8_t, 3>(color.xyz() * 255.0f), vm::vec<uint8_t, 1>(flags));
      vertDest[i++] = BrushEdgeVertex(
        pos2, vm::vec<uint8_t, 3>(color.xyz() * 255.0f), vm::vec<uint8_t, 1>(flags));
    }
  }

  // count vertices-per-face
  const size_t faceVerticesCount = [&]() {
    size_t total = 0u;
    for (const auto& face : brushValue.faces()) {
      total += face.vertexCount();
    }
    return total;
  }();

  // insert vertices into VBO, and also build a vector of face metadata
  std::vector<CachedFace> facesSortedByTex;
  facesSortedByTex.reserve(brushValue.faceCount());
  {
    assert(m_vertexArray != nullptr);
    auto [vertBlock, vertDest] = m_vertexArray->getPointerToInsertVerticesAt(faceVerticesCount);
    info.vertexHolderKey = vertBlock;

    size_t insertedVertices = 0u;
    const size_t vboRegionStart = vertBlock->pos;
    for (const Model::BrushFace& face : brushValue.faces()) {
      const size_t indexOfFirstVertex = vboRegionStart + insertedVertices;

      const auto faceNormal = vm::vec<int8_t, 3>(face.boundary().normal * 127.0);
      const auto flags = vm::vec<uint8_t, 1>(faceRenderFlags(brush, brushFlags, face));

      // The boundary is in CCW order, but the renderer expects CW order:
      auto& boundary = face.geometry()->boundary();
      for (auto it = std::rbegin(boundary), end = std::rend(boundary); it != end; ++it) {
        Model::BrushHalfEdge* current = *it;
        Model::BrushVertex* vertex = current->origin();

        vertDest[insertedVertices++] = BrushFaceVertex(
          vm::vec3f(vertex->position()), face.textureCoords(vertex->position()), faceNormal, flags);
      }

      // face cache
      facesSortedByTex.emplace_back(&face, indexOfFirstVertex);
    }

    assert(insertedVertices == faceVerticesCount);

    // Sort by texture so BrushRenderer can efficiently step through the BrushFaces
    // grouped by texture (via `BrushRendererBrushCache::cachedFacesSortedByTexture()`), without
    // needing to build an std::map

    std::sort(
      facesSortedByTex.begin(), facesSortedByTex.end(),
      [](const CachedFace& a, const CachedFace& b) {
        return a.texture < b.texture;
      });
  }

  // insert face indices
  const size_t facesSortedByTexSize = facesSortedByTex.size();

  size_t nextI;
  for (size_t i = 0; i < facesSortedByTexSize; i = nextI) {
    const Assets::Texture* texture = facesSortedByTex[i].texture;

    size_t opaqueIndexCount = 0;
    size_t transparentIndexCount = 0;

    // find the i value for the next texture
    for (nextI = i + 1; nextI < facesSortedByTexSize && facesSortedByTex[nextI].texture == texture;
         ++nextI) {}

    // process all faces with this texture (they'll be consecutive)
    for (size_t j = i; j < nextI; ++j) {
      const CachedFace& cache = facesSortedByTex[j];
      assert(cache.texture == texture);
      if (shouldDrawFaceInTransparentPass(brush, *cache.face)) {
        transparentIndexCount += triIndicesCountForPolygon(cache.vertexCount);
      } else {
        opaqueIndexCount += triIndicesCountForPolygon(cache.vertexCount);
      }
    }

    if (transparentIndexCount > 0) {
      TextureToBrushIndicesMap& faceVboMap = *m_transparentFaces;
      auto& holderPtr = faceVboMap[texture];
      if (holderPtr == nullptr) {
        // inserts into map!
        holderPtr = std::make_shared<BrushIndexArray>();
      }

      auto [key, insertDest] = holderPtr->getPointerToInsertElementsAt(transparentIndexCount);
      info.transparentFaceIndicesKeys.push_back({texture, key});

      // process all faces with this texture (they'll be consecutive)
      GLuint* currentDest = insertDest;
      for (size_t j = i; j < nextI; ++j) {
        const CachedFace& cache = facesSortedByTex[j];
        if (shouldDrawFaceInTransparentPass(brush, *cache.face)) {
          addTriIndicesForPolygon(
            currentDest, static_cast<GLuint>(cache.indexOfFirstVertex), cache.vertexCount);

          currentDest += triIndicesCountForPolygon(cache.vertexCount);
        }
      }
      assert(currentDest == (insertDest + transparentIndexCount));
    }

    if (opaqueIndexCount > 0) {
      TextureToBrushIndicesMap& faceVboMap = *m_opaqueFaces;
      auto& holderPtr = faceVboMap[texture];
      if (holderPtr == nullptr) {
        // inserts into map!
        holderPtr = std::make_shared<BrushIndexArray>();
      }

      auto [key, insertDest] = holderPtr->getPointerToInsertElementsAt(opaqueIndexCount);
      info.opaqueFaceIndicesKeys.push_back({texture, key});

      // process all faces with this texture (they'll be consecutive)
      GLuint* currentDest = insertDest;
      for (size_t j = i; j < nextI; ++j) {
        const CachedFace& cache = facesSortedByTex[j];
        if (!shouldDrawFaceInTransparentPass(brush, *cache.face)) {
          addTriIndicesForPolygon(
            currentDest, static_cast<GLuint>(cache.indexOfFirstVertex), cache.vertexCount);

          currentDest += triIndicesCountForPolygon(cache.vertexCount);
        }
      }
      assert(currentDest == (insertDest + opaqueIndexCount));
    }
  }
}

void BrushRenderer::addBrush(const Model::BrushNode* brush) {
  // i.e. insert the brush as "invalid" if it's not already present.
  // if it is present, its validity is unchanged.
  if (m_allBrushes.insert(brush).second) {
    assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
    assertResult(m_invalidBrushes.insert(brush).second);
  }
}

void BrushRenderer::removeBrush(const Model::BrushNode* brush) {
  // update m_brushValid
  m_allBrushes.erase(brush);

  if (m_invalidBrushes.erase(brush) > 0u) {
    // invalid brushes are not in the VBO, so we can return  now.
    assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
    return;
  }

  removeBrushFromVbo(brush);
}

void BrushRenderer::removeBrushFromVbo(const Model::BrushNode* brush) {
  auto it = m_brushInfo.find(brush);

  if (it == std::end(m_brushInfo)) {
    // This means BrushRenderer::validateBrush skipped rendering the brush, so it was never
    // uploaded to the VBO's
    return;
  }

  const BrushInfo& info = it->second;

  // update Vbo's
  m_vertexArray->deleteVerticesWithKey(info.vertexHolderKey, false);
  if (info.edgeVerticesKey != nullptr) {
    m_edgeVertices->deleteVerticesWithKey(info.edgeVerticesKey, true);
  }

  for (const auto& [texture, opaqueKey] : info.opaqueFaceIndicesKeys) {
    std::shared_ptr<BrushIndexArray> faceIndexHolder = m_opaqueFaces->at(texture);
    faceIndexHolder->zeroElementsWithKey(opaqueKey);

    if (!faceIndexHolder->hasValidIndices()) {
      // There are no indices left to render for this texture, so delete the <Texture,
      // BrushIndexArray> entry from the map
      m_opaqueFaces->erase(texture);
    }
  }
  for (const auto& [texture, transparentKey] : info.transparentFaceIndicesKeys) {
    std::shared_ptr<BrushIndexArray> faceIndexHolder = m_transparentFaces->at(texture);
    faceIndexHolder->zeroElementsWithKey(transparentKey);

    if (!faceIndexHolder->hasValidIndices()) {
      // There are no indices left to render for this texture, so delete the <Texture,
      // BrushIndexArray> entry from the map
      m_transparentFaces->erase(texture);
    }
  }

  m_brushInfo.erase(it);
}
} // namespace Renderer
} // namespace TrenchBroom
