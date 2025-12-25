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
#include "NotifierConnection.h"
#include "gl/ResourceId.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace tb
{
namespace mdl
{
class BrushNode;
class BrushFaceHandle;
class GroupNode;
class LayerNode;
class Map;
class Node;
struct SelectionChange;
} // namespace mdl

namespace render
{
class EntityDecalRenderer;
class EntityLinkRenderer;
class GroupLinkRenderer;
class ObjectRenderer;
class RenderBatch;
class RenderContext;

class MapRenderer
{
private:
  mdl::Map& m_map;

  std::unique_ptr<ObjectRenderer> m_defaultRenderer;
  std::unique_ptr<ObjectRenderer> m_selectionRenderer;
  std::unique_ptr<ObjectRenderer> m_lockedRenderer;
  std::unique_ptr<EntityDecalRenderer> m_entityDecalRenderer;
  std::unique_ptr<EntityLinkRenderer> m_entityLinkRenderer;
  std::unique_ptr<GroupLinkRenderer> m_groupLinkRenderer;

  enum class Renderer
  {
    Default = 1,
    Selection = 2,
    Locked = 4,
    All = Default | Selection | Locked
  };

  std::unordered_map<mdl::Node*, int> m_trackedNodes;

  NotifierConnection m_notifierConnection;

public:
  explicit MapRenderer(mdl::Map& map);
  ~MapRenderer();

  deleteCopyAndMove(MapRenderer);

public: // color config
  void overrideSelectionColors(const Color& color, float mix);
  void restoreSelectionColors();

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void setupGL(RenderBatch& renderBatch);
  void renderDefaultOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderDefaultTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderSelectionOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderSelectionTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderLockedOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderLockedTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderEntityDecals(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderGroupLinks(RenderContext& renderContext, RenderBatch& renderBatch);

  void setupRenderers();
  void setupDefaultRenderer(ObjectRenderer& renderer);
  void setupSelectionRenderer(ObjectRenderer& renderer);
  void setupLockedRenderer(ObjectRenderer& renderer);

  static int determineDesiredRenderers(mdl::Node& node);
  void updateAndInvalidateNode(mdl::Node& node);
  void updateAndInvalidateNodeRecursive(mdl::Node& node);
  void removeNode(mdl::Node& node);
  void removeNodeRecursive(mdl::Node& node);
  void updateAllNodes();

  void invalidateRenderers(Renderer renderers);
  void invalidateEntityDecalRenderer();
  void invalidateEntityLinkRenderer();
  void invalidateGroupLinkRenderer();
  void reloadEntityModels();

private: // notification
  void connectObservers();

  void nodesWereAdded(const std::vector<mdl::Node*>& nodes);
  void nodesWereRemoved(const std::vector<mdl::Node*>& nodes);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);

  void nodeVisibilityDidChange(const std::vector<mdl::Node*>& nodes);
  void nodeLockingDidChange(const std::vector<mdl::Node*>& nodes);

  void groupWasOpened();
  void groupWasClosed();

  void selectionDidChange(const mdl::SelectionChange& selectionChange);

  void resourcesWereProcessed(const std::vector<gl::ResourceId>& resourceIds);

  void materialCollectionsWillChange();
  void entityDefinitionsDidChange();
  void modsDidChange();

  void editorContextDidChange();

  void preferenceDidChange(const std::filesystem::path& path);
};

} // namespace render
} // namespace tb
