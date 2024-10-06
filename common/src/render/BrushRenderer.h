/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include "Color.h"
#include "Macros.h"
#include "mdl/BrushGeometry.h"
#include "render/AllocationTracker.h"
#include "render/EdgeRenderer.h"
#include "render/FaceRenderer.h"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tb::mdl
{
class BrushNode;
class BrushFace;
class EditorContext;
} // namespace tb::mdl

namespace tb::render
{

class BrushRenderer
{
public:
  class Filter
  {
  public:
    enum class FaceRenderPolicy
    {
      RenderMarked,
      RenderNone
    };

    enum class EdgeRenderPolicy
    {
      RenderAll,
      RenderIfEitherFaceMarked,
      RenderIfBothFacesMarked,
      RenderNone
    };

    using RenderSettings = std::tuple<FaceRenderPolicy, EdgeRenderPolicy>;

    Filter();
    virtual ~Filter();

    /**
     * Classifies whether the brush will be rendered, and which faces/edges.
     *
     * If both FaceRenderPolicy::RenderNone and EdgeRenderPolicy::RenderNone are returned,
     * the brush is skipped (not added to the vertex array or index arrays at all).
     *
     * Otherwise, markFaces() should call BrushFace::setMarked() on *all* faces, passing
     * true or false as needed to select the faces to be rendered.
     */
    virtual RenderSettings markFaces(const mdl::BrushNode& brush) const = 0;

  protected:
    /**
     * Return this from your markFaces() implementation to skip rendering of the brush.
     */
    static RenderSettings renderNothing();
  };

  class DefaultFilter : public Filter
  {
  private:
    const mdl::EditorContext& m_context;

  public:
    ~DefaultFilter() override;

  protected:
    explicit DefaultFilter(const mdl::EditorContext& context);

    bool visible(const mdl::BrushNode& brush) const;
    bool visible(const mdl::BrushNode& brush, const mdl::BrushFace& face) const;
    bool visible(const mdl::BrushNode& brush, const mdl::BrushEdge& edge) const;

    bool editable(const mdl::BrushNode& brush) const;
    bool editable(const mdl::BrushNode& brush, const mdl::BrushFace& face) const;

    bool selected(const mdl::BrushNode& brush) const;
    bool selected(const mdl::BrushNode& brush, const mdl::BrushFace& face) const;
    bool selected(const mdl::BrushNode& brush, const mdl::BrushEdge& edge) const;
    bool hasSelectedFaces(const mdl::BrushNode& brush) const;
  };

  class NoFilter : public Filter
  {
  public:
    using Filter::Filter;
    RenderSettings markFaces(const mdl::BrushNode& brushNode) const override;
  };

private:
  std::unique_ptr<Filter> m_filter;

  struct BrushInfo
  {
    AllocationTracker::Block* vertexHolderKey;
    AllocationTracker::Block* edgeIndicesKey;
    std::vector<std::pair<const mdl::Material*, AllocationTracker::Block*>>
      opaqueFaceIndicesKeys;
    std::vector<std::pair<const mdl::Material*, AllocationTracker::Block*>>
      transparentFaceIndicesKeys;
  };
  /**
   * Tracks all brushes that are stored in the VBO, with the information necessary to
   * remove them from the VBO later.
   */
  std::unordered_map<const mdl::BrushNode*, BrushInfo> m_brushInfo;

  /**
   * If a brush is in the VBO, it's always valid.
   * If a brush is valid, it might not be in the VBO if it was hidden by the Filter.
   *
   * Do not attempt to use vector_set here, it turns out to be slower.
   */
  std::unordered_set<const mdl::BrushNode*> m_allBrushes;
  std::unordered_set<const mdl::BrushNode*> m_invalidBrushes;

  std::shared_ptr<BrushVertexArray> m_vertexArray;
  std::shared_ptr<BrushIndexArray> m_edgeIndices;

  using MaterialToBrushIndicesMap =
    std::unordered_map<const mdl::Material*, std::shared_ptr<BrushIndexArray>>;
  std::shared_ptr<MaterialToBrushIndicesMap> m_transparentFaces;
  std::shared_ptr<MaterialToBrushIndicesMap> m_opaqueFaces;

  FaceRenderer m_opaqueFaceRenderer;
  FaceRenderer m_transparentFaceRenderer;
  IndexedEdgeRenderer m_edgeRenderer;

  Color m_faceColor;
  bool m_showEdges = false;
  Color m_edgeColor;
  bool m_grayscale = false;
  bool m_tint = false;
  Color m_tintColor;
  bool m_showOccludedEdges = false;
  Color m_occludedEdgeColor;
  bool m_forceTransparent = false;
  float m_transparencyAlpha = 1.0f;

  bool m_showHiddenBrushes = false;

public:
  template <typename FilterT>
  explicit BrushRenderer(FilterT filter)
    : m_filter{std::make_unique<FilterT>(std::move(filter))}
  {
    clear();
  }

  BrushRenderer();

  /**
   * Remove all brushes.
   */
  void clear();

  /**
   * Marks all of the brushes as invalid, meaning that next time one of the render()
   * methods is called,
   * - the Filter will be re-evaluated for each brush, possibly changing whether the brush
   * is included/excluded
   * - all brushes vertices will be re-fetched from the Brush object.
   *
   * Until a brush is invalidated, we don't re-evaluate the Filter, and don't check the
   * Brush object for modification.
   *
   * Additionally, calling `invalidate()` guarantees the m_brushInfo, m_transparentFaces,
   * and m_opaqueFaces maps will be empty, so the BrushRenderer will not have any
   * lingering Material* pointers.
   */
  void invalidate();
  void invalidateMaterials(const std::vector<const mdl::Material*>& materials);
  void invalidateBrush(const mdl::BrushNode* brush);
  void invalidateMaterial(const mdl::Material& material);
  bool valid() const;

  /**
   * Sets the color to render faces with no material with.
   */
  void setFaceColor(const Color& faceColor);

  /**
   * Specifies whether or not brush edges should be rendered.
   */
  void setShowEdges(bool showEdges);

  /**
   * The color to render brush edges with.
   */
  void setEdgeColor(const Color& edgeColor);

  /**
   * Specifies whether or not to render faces in grayscale.
   */
  void setGrayscale(bool grayscale);

  /**
   * Specifies whether or not to render faces with a tint.
   *
   * @see setTintColor
   */
  void setTint(bool tint);

  /**
   * Sets the color to tint faces with.
   */
  void setTintColor(const Color& tintColor);

  /**
   * Specifies whether or not occluded edges should be visible.
   */
  void setShowOccludedEdges(bool showOccludedEdges);

  /**
   * The color to render occluded edges with.
   */
  void setOccludedEdgeColor(const Color& occludedEdgeColor);

  /**
   * Specifies whether or not faces should be rendered transparent. Overrides any
   * transparency settings from the face itself or its material.
   *
   * Note: setTransparencyAlpha must be set to something less than 1.0 for this to have
   * any effect.
   *
   * @see setTransparencyAlpha
   */
  void setForceTransparent(bool transparent);

  /**
   * The alpha value to render transparent faces with.
   *
   * Note: this defaults to 1.0, which means requests for transparency from the brush,
   * face, or setForceTransparent() are ignored by default.
   */
  void setTransparencyAlpha(float transparencyAlpha);

  /**
   * Specifies whether or not brushes which are currently hidden should be rendered
   * regardless.
   */
  void setShowHiddenBrushes(bool showHiddenBrushes);

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void renderOpaqueFaces(RenderBatch& renderBatch);
  void renderTransparentFaces(RenderBatch& renderBatch);
  void renderEdges(RenderBatch& renderBatch);

public:
  /**
   * Only exposed for benchmarking.
   */
  void validate();

private:
  bool shouldDrawFaceInTransparentPass(
    const mdl::BrushNode& brushNode, const mdl::BrushFace& face) const;
  void validateBrush(const mdl::BrushNode& brushNode);

public:
  /**
   * Adds a brush. Calling with an already-added brush is allowed, but ignored (not
   * guaranteed to invalidate it).
   */
  void addBrush(const mdl::BrushNode* brushNode);
  /**
   * Removes a brush. Calling with an unknown brush is allowed, but ignored.
   */
  void removeBrush(const mdl::BrushNode* brushNode);

private:
  /**
   * If the given brush is not currently in the VBO, it's silently ignored.
   * Otherwise, it's removed from the VBO (having its indices zeroed out, causing it to no
   * longer draw). The brush's "valid" state is not touched inside here, but the
   * m_brushInfo is updated.
   */
  void removeBrushFromVbo(const mdl::BrushNode& brush);

  deleteCopyAndMove(BrushRenderer);
};

} // namespace tb::render
