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

#include "Notifier.h"
#include "mdl/TagType.h"

#include "kd/dynamic_bitset.h"

namespace tb::mdl
{
struct EntityDefinition;
class EntityNodeBase;
class BrushNode;
class BrushFace;
class EntityNode;
class GroupNode;
class LayerNode;
class Node;
class Object;
class PatchNode;
class WorldNode;

class EditorContext
{
private:
  TagType::Type m_hiddenTags = 0;
  kdl::dynamic_bitset m_hiddenEntityDefinitions;

  bool m_showPointEntities = true;
  bool m_showBrushes = true;

  bool m_blockSelection = false;

  LayerNode* m_currentLayer = nullptr;
  GroupNode* m_currentGroup = nullptr;

  bool m_alignmentLock = false;

public:
  Notifier<> editorContextDidChangeNotifier;

public:
  EditorContext();

  void reset();

  TagType::Type hiddenTags() const;
  void setHiddenTags(TagType::Type hiddenTags);

  bool entityDefinitionHidden(const EntityNodeBase& entityNode) const;
  bool entityDefinitionHidden(const EntityDefinition& definition) const;
  void setEntityDefinitionHidden(const EntityDefinition& definition, bool hidden);

  bool showPointEntities() const;
  void setShowPointEntities(bool showPointEntities);

  bool showBrushes() const;
  void setShowBrushes(bool showBrushes);

  bool blockSelection() const;
  void setBlockSelection(bool blockSelection);

public:
  LayerNode* currentLayer() const;
  void setCurrentLayer(LayerNode* layerNode);

  GroupNode* currentGroup() const;
  void pushGroup(GroupNode& groupNode);
  void popGroup();

public:
  bool alignmentLock() const;
  void setAlignmentLock(bool alignmentLock);

public:
  bool visible(const Node& node) const;
  bool visible(const WorldNode& worldNode) const;
  bool visible(const LayerNode& layerNode) const;
  bool visible(const GroupNode& groupNode) const;
  bool visible(const EntityNode& entityNode) const;
  bool visible(const BrushNode& brushNode) const;
  bool visible(const BrushNode& brushNode, const BrushFace& face) const;
  bool visible(const PatchNode& patchNode) const;

private:
  bool anyChildVisible(const Node& node) const;

public:
  bool editable(const Node& node) const;
  bool editable(const BrushNode& brushNode, const BrushFace& face) const;

  bool selectable(const Node& node) const;
  bool selectable(const WorldNode& worldNode) const;
  bool selectable(const LayerNode& layerNode) const;
  bool selectable(const GroupNode& groupNode) const;
  bool selectable(const EntityNode& entityNode) const;
  bool selectable(const BrushNode& brushNode) const;
  bool selectable(const BrushNode& brushNode, const BrushFace& face) const;
  bool selectable(const PatchNode& patchNode) const;

  bool canChangeSelection() const;
  bool inOpenGroup(const Object& object) const;

private:
  EditorContext(const EditorContext&);
  EditorContext& operator=(const EditorContext&);
};

} // namespace tb::mdl
