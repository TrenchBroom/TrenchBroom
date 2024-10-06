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

#include "kdl/bitset.h"

namespace tb::asset
{
class EntityDefinition;
}

namespace tb::mdl
{
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
  TagType::Type m_hiddenTags;
  kdl::bitset m_hiddenEntityDefinitions;

  bool m_blockSelection;

  mdl::GroupNode* m_currentGroup;

public:
  Notifier<> editorContextDidChangeNotifier;

public:
  EditorContext();

  void reset();

  TagType::Type hiddenTags() const;
  void setHiddenTags(TagType::Type hiddenTags);

  bool entityDefinitionHidden(const mdl::EntityNodeBase* entityNode) const;
  bool entityDefinitionHidden(const asset::EntityDefinition* definition) const;
  void setEntityDefinitionHidden(const asset::EntityDefinition* definition, bool hidden);

  bool blockSelection() const;
  void setBlockSelection(bool blockSelection);

public:
  mdl::GroupNode* currentGroup() const;
  void pushGroup(mdl::GroupNode* groupNode);
  void popGroup();

public:
  bool visible(const mdl::Node* node) const;
  bool visible(const mdl::WorldNode* worldNode) const;
  bool visible(const mdl::LayerNode* layerNode) const;
  bool visible(const mdl::GroupNode* groupNode) const;
  bool visible(const mdl::EntityNode* entityNode) const;
  bool visible(const mdl::BrushNode* brushNode) const;
  bool visible(const mdl::BrushNode* brushNode, const mdl::BrushFace& face) const;
  bool visible(const mdl::PatchNode* patchNode) const;

private:
  bool anyChildVisible(const mdl::Node* node) const;

public:
  bool editable(const mdl::Node* node) const;
  bool editable(const mdl::BrushNode* brushNode, const mdl::BrushFace& face) const;

  bool selectable(const mdl::Node* node) const;
  bool selectable(const mdl::WorldNode* worldNode) const;
  bool selectable(const mdl::LayerNode* layerNode) const;
  bool selectable(const mdl::GroupNode* groupNode) const;
  bool selectable(const mdl::EntityNode* entityNode) const;
  bool selectable(const mdl::BrushNode* brushNode) const;
  bool selectable(const mdl::BrushNode* brushNode, const mdl::BrushFace& face) const;
  bool selectable(const mdl::PatchNode* patchNode) const;

  bool canChangeSelection() const;
  bool inOpenGroup(const mdl::Object* object) const;

private:
  EditorContext(const EditorContext&);
  EditorContext& operator=(const EditorContext&);
};

} // namespace tb::mdl
