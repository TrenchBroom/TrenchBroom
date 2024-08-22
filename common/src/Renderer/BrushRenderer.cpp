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

#include "Assets/Material.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Polyhedron.h"
#include "Model/TagAttribute.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/BrushRendererBrushCache.h"
#include "Renderer/RenderContext.h"

#include <cassert>
#include <cstring>
#include <vector>

namespace TrenchBroom::Renderer
{

namespace
{

class FilterWrapper : public BrushRenderer::Filter
{
private:
  const Filter& m_filter;
  bool m_showHiddenBrushes;
  BrushRenderer::NoFilter m_noFilter;

  const Filter& resolve() const { return m_showHiddenBrushes ? m_noFilter : m_filter; }

public:
  FilterWrapper(const Filter& filter, const bool showHiddenBrushes)
    : m_filter{filter}
    , m_showHiddenBrushes{showHiddenBrushes}
  {
  }

  RenderSettings markFaces(const Model::BrushNode& brush) const override
  {
    return resolve().markFaces(brush);
  }
};

} // namespace

// Filter

BrushRenderer::Filter::Filter() = default;

BrushRenderer::Filter::~Filter() = default;

BrushRenderer::Filter::RenderSettings BrushRenderer::Filter::renderNothing()
{
  return std::tuple{FaceRenderPolicy::RenderNone, EdgeRenderPolicy::RenderNone};
}

// DefaultFilter

BrushRenderer::DefaultFilter::~DefaultFilter() = default;
BrushRenderer::DefaultFilter::DefaultFilter(const Model::EditorContext& context)
  : m_context{context}
{
}

bool BrushRenderer::DefaultFilter::visible(const Model::BrushNode& brush) const
{
  return m_context.visible(&brush);
}

bool BrushRenderer::DefaultFilter::visible(
  const Model::BrushNode& brush, const Model::BrushFace& face) const
{
  return m_context.visible(&brush, face);
}

bool BrushRenderer::DefaultFilter::visible(
  const Model::BrushNode& brushNode, const Model::BrushEdge& edge) const
{
  const auto& brush = brushNode.brush();
  const auto firstFaceIndex = edge.firstFace()->payload();
  const auto secondFaceIndex = edge.secondFace()->payload();
  assert(firstFaceIndex && secondFaceIndex);

  const auto& firstFace = brush.face(*firstFaceIndex);
  const auto& secondFace = brush.face(*secondFaceIndex);

  return m_context.visible(&brushNode, firstFace)
         || m_context.visible(&brushNode, secondFace);
}

bool BrushRenderer::DefaultFilter::editable(const Model::BrushNode& brush) const
{
  return m_context.editable(&brush);
}

bool BrushRenderer::DefaultFilter::editable(
  const Model::BrushNode& brush, const Model::BrushFace& face) const
{
  return m_context.editable(&brush, face);
}

bool BrushRenderer::DefaultFilter::selected(const Model::BrushNode& brush) const
{
  return brush.selected() || brush.parentSelected();
}

bool BrushRenderer::DefaultFilter::selected(
  const Model::BrushNode&, const Model::BrushFace& face) const
{
  return face.selected();
}

bool BrushRenderer::DefaultFilter::selected(
  const Model::BrushNode& brushNode, const Model::BrushEdge& edge) const
{
  const auto& brush = brushNode.brush();
  const auto firstFaceIndex = edge.firstFace()->payload();
  const auto secondFaceIndex = edge.secondFace()->payload();
  assert(firstFaceIndex && secondFaceIndex);

  const auto& firstFace = brush.face(*firstFaceIndex);
  const auto& secondFace = brush.face(*secondFaceIndex);

  return selected(brushNode) || selected(brushNode, firstFace)
         || selected(brushNode, secondFace);
}

bool BrushRenderer::DefaultFilter::hasSelectedFaces(const Model::BrushNode& brush) const
{
  return brush.descendantSelected();
}

// NoFilter

BrushRenderer::Filter::RenderSettings BrushRenderer::NoFilter::markFaces(
  const Model::BrushNode& brushNode) const
{
  const auto& brush = brushNode.brush();
  for (const auto& face : brush.faces())
  {
    face.setMarked(true);
  }
  return {FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderAll};
}

// BrushRenderer

BrushRenderer::BrushRenderer()
  : m_filter{std::make_unique<NoFilter>()}
{
  clear();
}

void BrushRenderer::invalidate()
{
  for (auto* brushNode : m_allBrushes)
  {
    // this will also invalidate already invalid brushes, which
    // is unnecessary
    removeBrushFromVbo(*brushNode);
  }
  m_invalidBrushes = m_allBrushes;

  assert(m_brushInfo.empty());
  assert(m_transparentFaces->empty());
  assert(m_opaqueFaces->empty());
}

void BrushRenderer::invalidateMaterials(
  const std::vector<const Assets::Material*>& materials)
{
  const auto materialSet =
    std::unordered_set<const Assets::Material*>{materials.begin(), materials.end()};
  for (auto* brush : m_allBrushes)
  {
    for (const auto& face : brush->brush().faces())
    {
      if (materialSet.count(face.material()) > 0)
      {
        brush->brushRendererBrushCache().invalidateVertexCache();
        invalidateBrush(brush);
      }
    }
  }
}

void BrushRenderer::invalidateBrush(const Model::BrushNode* brushNode)
{
  // skip brushes that are not in the renderer
  if (m_allBrushes.find(brushNode) == std::end(m_allBrushes))
  {
    assert(m_brushInfo.find(brushNode) == std::end(m_brushInfo));
    assert(m_invalidBrushes.find(brushNode) == std::end(m_invalidBrushes));
    return;
  }
  // if it's not in the invalid set, put it in
  if (m_invalidBrushes.insert(brushNode).second)
  {
    removeBrushFromVbo(*brushNode);
  }
}

bool BrushRenderer::valid() const
{
  return m_invalidBrushes.empty();
}

void BrushRenderer::clear()
{
  m_brushInfo.clear();
  m_allBrushes.clear();
  m_invalidBrushes.clear();

  m_vertexArray = std::make_shared<BrushVertexArray>();
  m_edgeIndices = std::make_shared<BrushIndexArray>();
  m_transparentFaces = std::make_shared<MaterialToBrushIndicesMap>();
  m_opaqueFaces = std::make_shared<MaterialToBrushIndicesMap>();

  m_opaqueFaceRenderer = FaceRenderer{m_vertexArray, m_opaqueFaces, m_faceColor};
  m_transparentFaceRenderer =
    FaceRenderer{m_vertexArray, m_transparentFaces, m_faceColor};
  m_edgeRenderer = IndexedEdgeRenderer{m_vertexArray, m_edgeIndices};
}

void BrushRenderer::setFaceColor(const Color& faceColor)
{
  m_faceColor = faceColor;
}

void BrushRenderer::setShowEdges(const bool showEdges)
{
  m_showEdges = showEdges;
}

void BrushRenderer::setEdgeColor(const Color& edgeColor)
{
  m_edgeColor = edgeColor;
}

void BrushRenderer::setGrayscale(const bool grayscale)
{
  m_grayscale = grayscale;
}

void BrushRenderer::setTint(const bool tint)
{
  m_tint = tint;
}

void BrushRenderer::setTintColor(const Color& tintColor)
{
  m_tintColor = tintColor;
}

void BrushRenderer::setShowOccludedEdges(const bool showOccludedEdges)
{
  m_showOccludedEdges = showOccludedEdges;
}

void BrushRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor)
{
  m_occludedEdgeColor = occludedEdgeColor;
}

void BrushRenderer::setForceTransparent(const bool transparent)
{
  if (transparent != m_forceTransparent)
  {
    m_forceTransparent = transparent;
    invalidate();
  }
}

void BrushRenderer::setTransparencyAlpha(const float transparencyAlpha)
{
  if (transparencyAlpha != m_transparencyAlpha)
  {
    m_transparencyAlpha = transparencyAlpha;
    invalidate();
  }
}

void BrushRenderer::setShowHiddenBrushes(const bool showHiddenBrushes)
{
  if (showHiddenBrushes != m_showHiddenBrushes)
  {
    m_showHiddenBrushes = showHiddenBrushes;
    invalidate();
  }
}

void BrushRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch)
{
  renderOpaque(renderContext, renderBatch);
  renderTransparent(renderContext, renderBatch);
}

void BrushRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!m_allBrushes.empty())
  {
    if (!valid())
    {
      validate();
    }
    if (renderContext.showFaces())
    {
      renderOpaqueFaces(renderBatch);
    }
    if (renderContext.showEdges() || m_showEdges)
    {
      renderEdges(renderBatch);
    }
  }
}

void BrushRenderer::renderTransparent(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!m_allBrushes.empty())
  {
    if (!valid())
    {
      validate();
    }
    if (renderContext.showFaces())
    {
      renderTransparentFaces(renderBatch);
    }
  }
}

void BrushRenderer::renderOpaqueFaces(RenderBatch& renderBatch)
{
  m_opaqueFaceRenderer.setGrayscale(m_grayscale);
  m_opaqueFaceRenderer.setTint(m_tint);
  m_opaqueFaceRenderer.setTintColor(m_tintColor);
  m_opaqueFaceRenderer.render(renderBatch);
}

void BrushRenderer::renderTransparentFaces(RenderBatch& renderBatch)
{
  m_transparentFaceRenderer.setGrayscale(m_grayscale);
  m_transparentFaceRenderer.setTint(m_tint);
  m_transparentFaceRenderer.setTintColor(m_tintColor);
  m_transparentFaceRenderer.setAlpha(m_transparencyAlpha);
  m_transparentFaceRenderer.render(renderBatch);
}

void BrushRenderer::renderEdges(RenderBatch& renderBatch)
{
  if (m_showOccludedEdges)
  {
    m_edgeRenderer.renderOnTop(renderBatch, m_occludedEdgeColor);
  }
  m_edgeRenderer.render(renderBatch, m_edgeColor);
}

void BrushRenderer::validate()
{
  assert(!valid());

  for (auto* brushNode : m_invalidBrushes)
  {
    validateBrush(*brushNode);
  }
  m_invalidBrushes.clear();
  assert(valid());

  m_opaqueFaceRenderer = FaceRenderer{m_vertexArray, m_opaqueFaces, m_faceColor};
  m_transparentFaceRenderer =
    FaceRenderer{m_vertexArray, m_transparentFaces, m_faceColor};
  m_edgeRenderer = IndexedEdgeRenderer{m_vertexArray, m_edgeIndices};
}

static size_t triIndicesCountForPolygon(const size_t vertexCount)
{
  assert(vertexCount >= 3);
  const size_t indexCount = 3 * (vertexCount - 2);
  return indexCount;
}

static void addTriIndicesForPolygon(
  GLuint* dest, const GLuint baseIndex, const size_t vertexCount)
{
  assert(vertexCount >= 3);
  for (size_t i = 0; i < vertexCount - 2; ++i)
  {
    *(dest++) = baseIndex;
    *(dest++) = baseIndex + static_cast<GLuint>(i + 1);
    *(dest++) = baseIndex + static_cast<GLuint>(i + 2);
  }
}

static inline bool shouldRenderEdge(
  const BrushRendererBrushCache::CachedEdge& edge,
  const BrushRenderer::Filter::EdgeRenderPolicy policy)
{
  using EdgeRenderPolicy = BrushRenderer::Filter::EdgeRenderPolicy;

  switch (policy)
  {
  case EdgeRenderPolicy::RenderAll:
    return true;
  case EdgeRenderPolicy::RenderIfEitherFaceMarked:
    return (edge.face1 && edge.face1->isMarked())
           || (edge.face2 && edge.face2->isMarked());
  case EdgeRenderPolicy::RenderIfBothFacesMarked:
    return (edge.face1 && edge.face1->isMarked())
           && (edge.face2 && edge.face2->isMarked());
  case EdgeRenderPolicy::RenderNone:
    return false;
    switchDefault();
  }
}

static size_t countMarkedEdgeIndices(
  const Model::BrushNode& brushNode, const BrushRenderer::Filter::EdgeRenderPolicy policy)
{
  using EdgeRenderPolicy = BrushRenderer::Filter::EdgeRenderPolicy;

  if (policy == EdgeRenderPolicy::RenderNone)
  {
    return 0;
  }

  size_t indexCount = 0;
  for (const auto& edge : brushNode.brushRendererBrushCache().cachedEdges())
  {
    if (shouldRenderEdge(edge, policy))
    {
      indexCount += 2;
    }
  }
  return indexCount;
}

static void getMarkedEdgeIndices(
  const Model::BrushNode& brushNode,
  const BrushRenderer::Filter::EdgeRenderPolicy policy,
  const GLuint brushVerticesStartIndex,
  GLuint* dest)
{
  using EdgeRenderPolicy = BrushRenderer::Filter::EdgeRenderPolicy;

  if (policy == EdgeRenderPolicy::RenderNone)
  {
    return;
  }

  size_t i = 0;
  for (const auto& edge : brushNode.brushRendererBrushCache().cachedEdges())
  {
    if (shouldRenderEdge(edge, policy))
    {
      dest[i++] =
        static_cast<GLuint>(brushVerticesStartIndex + edge.vertexIndex1RelativeToBrush);
      dest[i++] =
        static_cast<GLuint>(brushVerticesStartIndex + edge.vertexIndex2RelativeToBrush);
    }
  }
}

bool BrushRenderer::shouldDrawFaceInTransparentPass(
  const Model::BrushNode& brushNode, const Model::BrushFace& face) const
{
  if (m_transparencyAlpha >= 1.0f)
  {
    // In this case, draw everything in the opaque pass
    // see: https://github.com/TrenchBroom/TrenchBroom/issues/2848
    return false;
  }

  if (m_forceTransparent)
  {
    return true;
  }
  if (brushNode.hasAttribute(Model::TagAttributes::Transparency))
  {
    return true;
  }
  if (face.hasAttribute(Model::TagAttributes::Transparency))
  {
    return true;
  }
  return false;
}

void BrushRenderer::validateBrush(const Model::BrushNode& brushNode)
{
  assert(m_allBrushes.find(&brushNode) != std::end(m_allBrushes));
  assert(m_invalidBrushes.find(&brushNode) != std::end(m_invalidBrushes));
  assert(m_brushInfo.find(&brushNode) == std::end(m_brushInfo));

  const auto wrapper = FilterWrapper{*m_filter, m_showHiddenBrushes};

  // evaluate filter. only evaluate the filter once per brush.
  const auto settings = wrapper.markFaces(brushNode);
  const auto [facePolicy, edgePolicy] = settings;

  if (
    facePolicy == Filter::FaceRenderPolicy::RenderNone
    && edgePolicy == Filter::EdgeRenderPolicy::RenderNone)
  {
    // NOTE: this skips inserting the brush into m_brushInfo
    return;
  }

  BrushInfo& info = m_brushInfo[&brushNode];

  // collect vertices
  auto& brushCache = brushNode.brushRendererBrushCache();
  brushCache.validateVertexCache(brushNode);
  const auto& cachedVertices = brushCache.cachedVertices();
  ensure(!cachedVertices.empty(), "Brush must have cached vertices");

  assert(m_vertexArray != nullptr);
  auto [vertBlock, dest] =
    m_vertexArray->getPointerToInsertVerticesAt(cachedVertices.size());
  std::memcpy(dest, cachedVertices.data(), cachedVertices.size() * sizeof(*dest));
  info.vertexHolderKey = vertBlock;

  const auto brushVerticesStartIndex = static_cast<GLuint>(vertBlock->pos);

  // insert edge indices into VBO
  {
    const auto edgeIndexCount = countMarkedEdgeIndices(brushNode, edgePolicy);
    if (edgeIndexCount > 0)
    {
      auto [key, insertDest] =
        m_edgeIndices->getPointerToInsertElementsAt(edgeIndexCount);
      info.edgeIndicesKey = key;
      getMarkedEdgeIndices(brushNode, edgePolicy, brushVerticesStartIndex, insertDest);
    }
    else
    {
      // it's possible to have no edges to render
      // e.g. select all faces of a brush, and the unselected brush renderer
      // will hit this branch.
      ensure(info.edgeIndicesKey == nullptr, "BrushInfo not initialized");
    }
  }

  // insert face indices

  auto& facesSortedByMaterial = brushCache.cachedFacesSortedByMaterial();
  const auto facesSortedByMaterialCount = facesSortedByMaterial.size();

  size_t nextI;
  for (size_t i = 0; i < facesSortedByMaterialCount; i = nextI)
  {
    const auto* material = facesSortedByMaterial[i].material;

    size_t opaqueIndexCount = 0;
    size_t transparentIndexCount = 0;

    // find the i value for the next material
    for (nextI = i + 1; nextI < facesSortedByMaterialCount
                        && facesSortedByMaterial[nextI].material == material;
         ++nextI)
    {
    }

    // process all faces with this material (they'll be consecutive)
    for (size_t j = i; j < nextI; ++j)
    {
      const auto& cache = facesSortedByMaterial[j];
      if (cache.face->isMarked())
      {
        assert(cache.material == material);
        if (shouldDrawFaceInTransparentPass(brushNode, *cache.face))
        {
          transparentIndexCount += triIndicesCountForPolygon(cache.vertexCount);
        }
        else
        {
          opaqueIndexCount += triIndicesCountForPolygon(cache.vertexCount);
        }
      }
    }

    if (transparentIndexCount > 0)
    {
      auto& faceVboMap = *m_transparentFaces;
      auto& holderPtr = faceVboMap[material];
      if (holderPtr == nullptr)
      {
        // inserts into map!
        holderPtr = std::make_shared<BrushIndexArray>();
      }

      auto [key, insertDest] =
        holderPtr->getPointerToInsertElementsAt(transparentIndexCount);
      info.transparentFaceIndicesKeys.emplace_back(material, key);

      // process all faces with this material (they'll be consecutive)
      auto* currentDest = insertDest;
      for (size_t j = i; j < nextI; ++j)
      {
        const auto& cache = facesSortedByMaterial[j];
        if (
          cache.face->isMarked()
          && shouldDrawFaceInTransparentPass(brushNode, *cache.face))
        {
          addTriIndicesForPolygon(
            currentDest,
            static_cast<GLuint>(
              brushVerticesStartIndex + cache.indexOfFirstVertexRelativeToBrush),
            cache.vertexCount);

          currentDest += triIndicesCountForPolygon(cache.vertexCount);
        }
      }
      assert(currentDest == (insertDest + transparentIndexCount));
    }

    if (opaqueIndexCount > 0)
    {
      auto& faceVboMap = *m_opaqueFaces;
      auto& holderPtr = faceVboMap[material];
      if (holderPtr == nullptr)
      {
        // inserts into map!
        holderPtr = std::make_shared<BrushIndexArray>();
      }

      auto [key, insertDest] = holderPtr->getPointerToInsertElementsAt(opaqueIndexCount);
      info.opaqueFaceIndicesKeys.emplace_back(material, key);

      // process all faces with this material (they'll be consecutive)
      auto* currentDest = insertDest;
      for (size_t j = i; j < nextI; ++j)
      {
        const auto& cache = facesSortedByMaterial[j];
        if (
          cache.face->isMarked()
          && !shouldDrawFaceInTransparentPass(brushNode, *cache.face))
        {
          addTriIndicesForPolygon(
            currentDest,
            static_cast<GLuint>(
              brushVerticesStartIndex + cache.indexOfFirstVertexRelativeToBrush),
            cache.vertexCount);

          currentDest += triIndicesCountForPolygon(cache.vertexCount);
        }
      }
      assert(currentDest == (insertDest + opaqueIndexCount));
    }
  }
}

void BrushRenderer::addBrush(const Model::BrushNode* brushNode)
{
  // i.e. insert the brush as "invalid" if it's not already present.
  // if it is present, its validity is unchanged.
  if (m_allBrushes.insert(brushNode).second)
  {
    assert(m_brushInfo.find(brushNode) == std::end(m_brushInfo));
    assertResult(m_invalidBrushes.insert(brushNode).second);
  }
}

void BrushRenderer::removeBrush(const Model::BrushNode* brushNode)
{
  // update m_brushValid
  m_allBrushes.erase(brushNode);

  if (m_invalidBrushes.erase(brushNode) > 0u)
  {
    // invalid brushes are not in the VBO, so we can return  now.
    assert(m_brushInfo.find(brushNode) == std::end(m_brushInfo));
    return;
  }

  removeBrushFromVbo(*brushNode);
}

void BrushRenderer::removeBrushFromVbo(const Model::BrushNode& brushNode)
{
  auto it = m_brushInfo.find(&brushNode);

  if (it == std::end(m_brushInfo))
  {
    // This means BrushRenderer::validateBrush skipped rendering the brush, so it was
    // never uploaded to the VBO's
    return;
  }

  const auto& info = it->second;

  // update Vbo's
  m_vertexArray->deleteVerticesWithKey(info.vertexHolderKey);
  if (info.edgeIndicesKey != nullptr)
  {
    m_edgeIndices->zeroElementsWithKey(info.edgeIndicesKey);
  }

  for (const auto& [material, opaqueKey] : info.opaqueFaceIndicesKeys)
  {
    auto faceIndexHolder = m_opaqueFaces->at(material);
    faceIndexHolder->zeroElementsWithKey(opaqueKey);

    if (!faceIndexHolder->hasValidIndices())
    {
      // There are no indices left to render for this material, so delete the <Material,
      // BrushIndexArray> entry from the map
      m_opaqueFaces->erase(material);
    }
  }
  for (const auto& [material, transparentKey] : info.transparentFaceIndicesKeys)
  {
    auto faceIndexHolder = m_transparentFaces->at(material);
    faceIndexHolder->zeroElementsWithKey(transparentKey);

    if (!faceIndexHolder->hasValidIndices())
    {
      // There are no indices left to render for this material, so delete the <Material,
      // BrushIndexArray> entry from the map
      m_transparentFaces->erase(material);
    }
  }

  m_brushInfo.erase(it);
}

} // namespace TrenchBroom::Renderer
