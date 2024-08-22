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

#pragma once

#include "Color.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/MaterialIndexArrayRenderer.h"
#include "Renderer/Renderable.h"

#include "kdl/vector_set.h"

#include <vector>

namespace TrenchBroom::Model
{
class EditorContext;
class PatchNode;
} // namespace TrenchBroom::Model

namespace TrenchBroom::Renderer
{
class RenderBatch;
class RenderContext;
class VboManager;

class PatchRenderer : public IndexedRenderable
{
private:
  const Model::EditorContext& m_editorContext;

  bool m_valid = true;
  kdl::vector_set<const Model::PatchNode*> m_patchNodes;

  MaterialIndexArrayRenderer m_patchMeshRenderer;
  DirectEdgeRenderer m_edgeRenderer;

  Color m_defaultColor;
  bool m_grayscale = false;
  bool m_tint = false;
  Color m_tintColor;
  float m_alpha = 1.0f;

  bool m_showEdges = true;
  Color m_edgeColor;
  bool m_showOccludedEdges = false;
  Color m_occludedEdgeColor;

public:
  explicit PatchRenderer(const Model::EditorContext& editorContext);

  void setDefaultColor(const Color& faceColor);
  void setGrayscale(bool grayscale);
  void setTint(bool tint);
  void setTintColor(const Color& color);
  void setTransparencyAlpha(float alpha);

  /**
   * Specifies whether or not patch edges should be rendered.
   */
  void setShowEdges(bool showEdges);

  /**
   * The color to render patch edges with.
   */
  void setEdgeColor(const Color& edgeColor);

  /**
   * Specifies whether or not occluded edges should be visible.
   */
  void setShowOccludedEdges(bool showOccludedEdges);

  /**
   * The color to render occluded edges with.
   */
  void setOccludedEdgeColor(const Color& occludedEdgeColor);

  /**
   * Equivalent to invalidatePatch() on all added patches.
   */
  void invalidate();
  /**
   * Equivalent to removePatch() on all added patches.
   */
  void clear();

  /**
   * Adds a patch. Calling with an already-added patch is allowed, but ignored (not
   * guaranteed to invalidate it).
   */
  void addPatch(const Model::PatchNode* patchNode);
  /**
   * Removes a patch. Calling with an unknown patch is allowed, but ignored.
   */
  void removePatch(const Model::PatchNode* patchNode);
  /**
   * Causes cached renderer data to be rebuilt for the given patch (on the next render()
   * call).
   */
  void invalidatePatch(const Model::PatchNode* patchNode);

  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void validate();

private: // implement IndexedRenderable interface
  void prepareVerticesAndIndices(VboManager& vboManager) override;
  void doRender(RenderContext& renderContext) override;
};

} // namespace TrenchBroom::Renderer
