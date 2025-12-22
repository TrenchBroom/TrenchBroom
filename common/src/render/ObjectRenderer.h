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
#include "render/BrushRenderer.h"
#include "render/EntityRenderer.h"
#include "render/GroupRenderer.h"
#include "render/PatchRenderer.h"

#include <vector>

namespace tb
{
class Logger;

namespace gl
{
class Material;
}

namespace mdl
{
class BrushNode;
class EditorContext;
class EntityModel;
class EntityModelManager;
class EntityNode;
class GroupNode;
class Node;
class PatchNode;
} // namespace mdl

namespace render
{
class FontManager;
class RenderBatch;

class ObjectRenderer
{
private:
  GroupRenderer m_groupRenderer;
  EntityRenderer m_entityRenderer;
  BrushRenderer m_brushRenderer;
  PatchRenderer m_patchRenderer;

public:
  template <typename BrushFilterT>
  ObjectRenderer(
    Logger& logger,
    mdl::EntityModelManager& entityModelManager,
    const mdl::EditorContext& editorContext,
    const BrushFilterT& brushFilter)
    : m_groupRenderer{editorContext}
    , m_entityRenderer{logger, entityModelManager, editorContext}
    , m_brushRenderer{brushFilter}
    , m_patchRenderer{editorContext}
  {
  }

public: // object management
  void addNode(mdl::Node& node);
  void removeNode(mdl::Node& node);
  void invalidateMaterials(const std::vector<const gl::Material*>& materials);
  void invalidateEntityModels(const std::vector<const mdl::EntityModel*>& entityModels);
  void invalidateNode(mdl::Node& node);
  void invalidate();
  void clear();
  void reloadModels();

public: // configuration
  void setShowOverlays(bool showOverlays);
  void setEntityOverlayTextColor(const Color& overlayTextColor);
  void setGroupOverlayTextColor(const Color& overlayTextColor);
  void setOverlayBackgroundColor(const Color& overlayBackgroundColor);

  void setTint(bool tint);
  void setTintColor(const Color& tintColor);

  void setShowOccludedObjects(bool showOccludedObjects);
  void setOccludedEdgeColor(const Color& occludedEdgeColor);

  void setTransparencyAlpha(float transparencyAlpha);

  void setShowEntityAngles(bool showAngles);
  void setEntityAngleColor(const Color& color);

  void setOverrideGroupColors(bool overrideGroupColors);
  void setGroupBoundsColor(const Color& color);

  void setOverrideEntityBoundsColor(bool overrideEntityBoundsColor);
  void setEntityBoundsColor(const Color& color);

  void setShowBrushEdges(bool showBrushEdges);
  void setBrushFaceColor(const Color& brushFaceColor);
  void setBrushEdgeColor(const Color& brushEdgeColor);

  void setShowHiddenObjects(bool showHiddenObjects);

public: // rendering
  void renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch);

  deleteCopy(ObjectRenderer);
};

} // namespace render
} // namespace tb
