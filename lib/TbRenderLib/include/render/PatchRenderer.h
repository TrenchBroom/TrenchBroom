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
#include "gl/MaterialIndexArrayRenderer.h"
#include "render/EdgeRenderer.h"
#include "render/Renderable.h"

#include "kd/vector_set.h"

namespace tb
{
namespace mdl
{
class EditorContext;
class PatchNode;
} // namespace mdl

namespace render
{
class RenderBatch;
class RenderContext;

class PatchRenderer : public IndexedRenderable
{
private:
  const mdl::EditorContext& m_editorContext;

  bool m_valid = true;
  kdl::vector_set<const mdl::PatchNode*> m_patchNodes;

  gl::MaterialIndexArrayRenderer m_patchMeshRenderer;
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
  explicit PatchRenderer(const mdl::EditorContext& editorContext);

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
  void addPatch(const mdl::PatchNode* patchNode);
  /**
   * Removes a patch. Calling with an unknown patch is allowed, but ignored.
   */
  void removePatch(const mdl::PatchNode* patchNode);
  /**
   * Causes cached renderer data to be rebuilt for the given patch (on the next render()
   * call).
   */
  void invalidatePatch(const mdl::PatchNode* patchNode);

  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void validate();

private: // implement IndexedRenderable interface
  void prepareVerticesAndIndices(gl::VboManager& vboManager) override;
  void render(RenderContext& renderContext) override;
};

} // namespace render
} // namespace tb
