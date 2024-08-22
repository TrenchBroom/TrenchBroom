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

#pragma once

#include "Color.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/TriangleRenderer.h"

#include "kdl/vector_set.h"

#include "vm/forward.h"

#include <vector>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class EntityModel;
class EntityModelManager;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
class EditorContext;
class EntityNode;
} // namespace TrenchBroom::Model

namespace TrenchBroom::Renderer
{
class AttrString;

class EntityRenderer
{
private:
  Assets::EntityModelManager& m_entityModelManager;
  const Model::EditorContext& m_editorContext;
  kdl::vector_set<const Model::EntityNode*> m_entities;

  DirectEdgeRenderer m_pointEntityWireframeBoundsRenderer;
  DirectEdgeRenderer m_brushEntityWireframeBoundsRenderer;

  TriangleRenderer m_solidBoundsRenderer;
  EntityModelRenderer m_modelRenderer;
  bool m_boundsValid = false;

  bool m_showOverlays = true;
  Color m_overlayTextColor;
  Color m_overlayBackgroundColor;
  bool m_showOccludedOverlays = false;
  bool m_tint = false;
  Color m_tintColor;
  bool m_overrideBoundsColor = false;
  Color m_boundsColor;
  bool m_showOccludedBounds = false;
  Color m_occludedBoundsColor;
  bool m_showAngles = false;
  Color m_angleColor;
  bool m_showHiddenEntities = false;

public:
  EntityRenderer(
    Logger& logger,
    Assets::EntityModelManager& entityModelManager,
    const Model::EditorContext& editorContext);

  /**
   * Equivalent to invalidateEntity() on all added entities.
   */
  void invalidate();
  /**
   * Equivalent to removeEntity() on all added entities.
   */
  void clear();
  void reloadModels();

  /**
   * Adds an entity. Calling with an already-added entity is allowed, but ignored (not
   * guaranteed to invalidate it).
   */
  void addEntity(const Model::EntityNode* entity);
  /**
   * Removes an entity. Calling with an unknown entity is allowed, but ignored.
   */
  void removeEntity(const Model::EntityNode* entity);
  /**
   * Causes cached renderer data to be rebuilt for the given entity (on the next render()
   * call).
   */
  void invalidateEntity(const Model::EntityNode* entity);

  /**
   * Invalidates cached renderer data to be result for any entity that references any of
   * the given models.
   */
  void invalidateEntityModels(
    const std::vector<const Assets::EntityModel*>& entityModels);

  void setShowOverlays(bool showOverlays);
  void setOverlayTextColor(const Color& overlayTextColor);
  void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
  void setShowOccludedOverlays(bool showOccludedOverlays);

  void setTint(bool tint);
  void setTintColor(const Color& tintColor);

  void setOverrideBoundsColor(bool overrideBoundsColor);
  void setBoundsColor(const Color& boundsColor);

  void setShowOccludedBounds(bool showOccludedBounds);
  void setOccludedBoundsColor(const Color& occludedBoundsColor);

  void setShowAngles(bool showAngles);
  void setAngleColor(const Color& angleColor);

  void setShowHiddenEntities(bool showHiddenEntities);

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void renderBounds(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderPointEntityWireframeBounds(RenderBatch& renderBatch);
  void renderBrushEntityWireframeBounds(RenderBatch& renderBatch);
  void renderSolidBounds(RenderBatch& renderBatch);
  void renderModels(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderClassnames(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderAngles(RenderContext& renderContext, RenderBatch& renderBatch);
  std::vector<vm::vec3f> arrowHead(float length, float width) const;

  void invalidateBounds();
  void validateBounds();

  AttrString entityString(const Model::EntityNode* entityNode) const;
  const Color& boundsColor(const Model::EntityNode* entityNode) const;
};

} // namespace TrenchBroom::Renderer
