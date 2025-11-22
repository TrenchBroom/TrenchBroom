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

#include "AttrString.h"
#include "Color.h"
#include "render/EdgeRenderer.h"

#include "kd/vector_set.h"

namespace tb
{
namespace mdl
{
class EditorContext;
class GroupNode;
} // namespace mdl

namespace render
{
class RenderBatch;
class RenderContext;

class GroupRenderer
{
private:
  class GroupNameAnchor;

  const mdl::EditorContext& m_editorContext;
  kdl::vector_set<const mdl::GroupNode*> m_groups;

  DirectEdgeRenderer m_boundsRenderer;
  bool m_boundsValid = false;

  bool m_overrideColors = false;
  bool m_showOverlays = true;
  Color m_overlayTextColor;
  Color m_overlayBackgroundColor;
  bool m_showOccludedOverlays = false;
  Color m_boundsColor;
  bool m_showOccludedBounds = false;
  Color m_occludedBoundsColor;

public:
  explicit GroupRenderer(const mdl::EditorContext& editorContext);

  /**
   * Equivalent to invalidateGroup() on all added groups.
   */
  void invalidate();
  /**
   * Equivalent to removeGroup() on all added groups.
   */
  void clear();

  /**
   * Adds a group. Calling with an already-added group is allowed, but ignored (not
   * guaranteed to invalidate it).
   */
  void addGroup(const mdl::GroupNode* group);
  /**
   * Removes a group. Calling with an unknown group is allowed, but ignored.
   */
  void removeGroup(const mdl::GroupNode* group);
  /**
   * Causes cached renderer data to be rebuilt for the given group (on the next render()
   * call).
   */
  void invalidateGroup(const mdl::GroupNode* group);

  void setOverrideColors(bool overrideColors);

  void setShowOverlays(bool showOverlays);
  void setOverlayTextColor(const Color& overlayTextColor);
  void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
  void setShowOccludedOverlays(bool showOccludedOverlays);

  void setBoundsColor(const Color& boundsColor);

  void setShowOccludedBounds(bool showOccludedBounds);
  void setOccludedBoundsColor(const Color& occludedBoundsColor);

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void renderBounds(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderNames(RenderContext& renderContext, RenderBatch& renderBatch);

  void invalidateBounds();
  void validateBounds();

  bool shouldRenderGroup(const mdl::GroupNode& group) const;

  AttrString groupString(const mdl::GroupNode& group) const;
  Color groupColor(const mdl::GroupNode& group) const;
};

} // namespace render
} // namespace tb
