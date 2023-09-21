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

#include "Macros.h"
#include "NotifierConnection.h"

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace TrenchBroom
{
class Color;

namespace View
{
// FIXME: Renderer should not depend on View
class MapDocument;
class Selection;
} // namespace View

namespace Model
{
class BrushNode;
class BrushFaceHandle;
class GroupNode;
class LayerNode;
class Node;
} // namespace Model

namespace Renderer
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
  std::weak_ptr<View::MapDocument> m_document;

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

  std::unordered_map<Model::Node*, int> m_trackedNodes;

  NotifierConnection m_notifierConnection;

public:
  explicit MapRenderer(std::weak_ptr<View::MapDocument> document);
  ~MapRenderer();

  deleteCopyAndMove(MapRenderer);

private:
  static std::unique_ptr<ObjectRenderer> createDefaultRenderer(
    std::weak_ptr<View::MapDocument> document);
  static std::unique_ptr<ObjectRenderer> createSelectionRenderer(
    std::weak_ptr<View::MapDocument> document);
  static std::unique_ptr<ObjectRenderer> createLockRenderer(
    std::weak_ptr<View::MapDocument> document);
  static std::unique_ptr<EntityDecalRenderer> createEntityDecalRenderer(
    std::weak_ptr<View::MapDocument> document);
  void clear();

public: // color config
  void overrideSelectionColors(const Color& color, float mix);
  void restoreSelectionColors();

public: // rendering
  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void commitPendingChanges();
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

  static int determineDesiredRenderers(Model::Node* node);
  void updateAndInvalidateNode(Model::Node* node);
  void updateAndInvalidateNodeRecursive(Model::Node* node);
  void removeNode(Model::Node* node);
  void removeNodeRecursive(Model::Node* node);
  void updateAllNodes();

  void invalidateRenderers(Renderer renderers);
  void invalidateEntityDecalRenderer();
  void invalidateEntityLinkRenderer();
  void invalidateGroupLinkRenderer();
  void reloadEntityModels();

private: // notification
  void connectObservers();

  void documentWasCleared(View::MapDocument* document);
  void documentWasNewedOrLoaded(View::MapDocument* document);

  void nodesWereAdded(const std::vector<Model::Node*>& nodes);
  void nodesWereRemoved(const std::vector<Model::Node*>& nodes);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);

  void nodeVisibilityDidChange(const std::vector<Model::Node*>& nodes);
  void nodeLockingDidChange(const std::vector<Model::Node*>& nodes);

  void groupWasOpened(Model::GroupNode* group);
  void groupWasClosed(Model::GroupNode* group);

  void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);

  void selectionDidChange(const View::Selection& selection);

  void textureCollectionsWillChange();
  void entityDefinitionsDidChange();
  void modsDidChange();

  void editorContextDidChange();

  void preferenceDidChange(const std::filesystem::path& path);
};
} // namespace Renderer
} // namespace TrenchBroom
