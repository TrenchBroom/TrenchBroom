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

#include "EditorContext.h"

#include "Ensure.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

EditorContext::EditorContext()
{
  reset();
}

void EditorContext::reset()
{
  m_hiddenTags = 0;
  m_hiddenEntityDefinitions.reset();
  m_blockSelection = false;
  m_currentGroup = nullptr;
}

TagType::Type EditorContext::hiddenTags() const
{
  return m_hiddenTags;
}

void EditorContext::setHiddenTags(const TagType::Type hiddenTags)
{
  if (hiddenTags != m_hiddenTags)
  {
    m_hiddenTags = hiddenTags;
    editorContextDidChangeNotifier();
  }
}

bool EditorContext::entityDefinitionHidden(const mdl::EntityNodeBase* entityNode) const
{
  return entityNode && entityNode->entity().definition()
         && entityDefinitionHidden(*entityNode->entity().definition());
}

bool EditorContext::entityDefinitionHidden(const EntityDefinition& definition) const
{
  return m_hiddenEntityDefinitions[definition.index];
}

void EditorContext::setEntityDefinitionHidden(
  const EntityDefinition& definition, const bool hidden)
{
  if (entityDefinitionHidden(definition) != hidden)
  {
    m_hiddenEntityDefinitions[definition.index] = hidden;
    editorContextDidChangeNotifier();
  }
}

bool EditorContext::blockSelection() const
{
  return m_blockSelection;
}

void EditorContext::setBlockSelection(const bool blockSelection)
{
  if (m_blockSelection != blockSelection)
  {
    m_blockSelection = blockSelection;
    editorContextDidChangeNotifier();
  }
}

mdl::GroupNode* EditorContext::currentGroup() const
{
  return m_currentGroup;
}

void EditorContext::pushGroup(mdl::GroupNode* groupNode)
{
  ensure(groupNode, "group is null");
  assert(!m_currentGroup || groupNode->containingGroup() == m_currentGroup);

  if (m_currentGroup)
  {
    m_currentGroup->close();
  }
  m_currentGroup = groupNode;
  m_currentGroup->open();
}

void EditorContext::popGroup()
{
  ensure(m_currentGroup, "currentGroup is null");

  m_currentGroup->close();
  m_currentGroup = m_currentGroup->containingGroup();
  if (m_currentGroup)
  {
    m_currentGroup->open();
  }
}

bool EditorContext::visible(const mdl::Node* node) const
{
  return node->accept(kdl::overload(
    [&](const WorldNode* worldNode) { return visible(worldNode); },
    [&](const LayerNode* layerNode) { return visible(layerNode); },
    [&](const GroupNode* groupNode) { return visible(groupNode); },
    [&](const EntityNode* entityNode) { return visible(entityNode); },
    [&](const BrushNode* brushNode) { return visible(brushNode); },
    [&](const PatchNode* patchNode) { return visible(patchNode); }));
}

bool EditorContext::visible(const mdl::WorldNode* worldNode) const
{
  return worldNode->visible();
}

bool EditorContext::visible(const mdl::LayerNode* layerNode) const
{
  return layerNode->visible();
}

bool EditorContext::visible(const mdl::GroupNode* groupNode) const
{
  if (groupNode->selected())
  {
    return true;
  }
  if (!anyChildVisible(groupNode))
  {
    return false;
  }
  return groupNode->visible();
}

bool EditorContext::visible(const mdl::EntityNode* entityNode) const
{
  if (entityNode->selected())
  {
    return true;
  }

  if (!entityNode->entity().pointEntity())
  {
    return anyChildVisible(entityNode);
  }

  if (!entityNode->visible())
  {
    return false;
  }

  if (entityNode->entity().pointEntity() && !pref(Preferences::ShowPointEntities))
  {
    return false;
  }

  if (entityDefinitionHidden(entityNode))
  {
    return false;
  }

  return true;
}

bool EditorContext::visible(const mdl::BrushNode* brushNode) const
{
  if (brushNode->selected())
  {
    return true;
  }

  if (!pref(Preferences::ShowBrushes))
  {
    return false;
  }

  if (brushNode->hasTag(m_hiddenTags))
  {
    return false;
  }

  if (brushNode->allFacesHaveAnyTagInMask(m_hiddenTags))
  {
    return false;
  }

  if (entityDefinitionHidden(brushNode->entity()))
  {
    return false;
  }

  return brushNode->visible();
}

bool EditorContext::visible(
  const mdl::BrushNode* brushNode, const mdl::BrushFace& face) const
{
  return visible(brushNode) && !face.hasTag(m_hiddenTags);
}

bool EditorContext::visible(const mdl::PatchNode* patchNode) const
{
  if (patchNode->selected())
  {
    return true;
  }

  if (patchNode->hasTag(m_hiddenTags))
  {
    return false;
  }

  return patchNode->visible();
}

bool EditorContext::anyChildVisible(const mdl::Node* node) const
{
  const auto& children = node->children();
  return std::ranges::any_of(
    children, [this](const Node* child) { return visible(child); });
}

bool EditorContext::editable(const mdl::Node* node) const
{
  return node->editable();
}

bool EditorContext::editable(const mdl::BrushNode* brushNode, const mdl::BrushFace&) const
{
  return editable(brushNode);
}

bool EditorContext::selectable(const mdl::Node* node) const
{
  return node->accept(kdl::overload(
    [&](const WorldNode* worldNode) { return selectable(worldNode); },
    [&](const LayerNode* layerNode) { return selectable(layerNode); },
    [&](const GroupNode* groupNode) { return selectable(groupNode); },
    [&](const EntityNode* entityNode) { return selectable(entityNode); },
    [&](const BrushNode* brushNode) { return selectable(brushNode); },
    [&](const PatchNode* patchNode) { return selectable(patchNode); }));
}

bool EditorContext::selectable(const mdl::WorldNode*) const
{
  return false;
}

bool EditorContext::selectable(const mdl::LayerNode*) const
{
  return false;
}

bool EditorContext::selectable(const mdl::GroupNode* groupNode) const
{
  return visible(groupNode) && editable(groupNode) && !groupNode->opened()
         && inOpenGroup(groupNode);
}

bool EditorContext::selectable(const mdl::EntityNode* entityNode) const
{
  return visible(entityNode) && editable(entityNode) && !entityNode->hasChildren()
         && inOpenGroup(entityNode);
}

bool EditorContext::selectable(const mdl::BrushNode* brushNode) const
{
  return visible(brushNode) && editable(brushNode) && inOpenGroup(brushNode);
}

bool EditorContext::selectable(
  const mdl::BrushNode* brushNode, const mdl::BrushFace& face) const
{
  return visible(brushNode, face) && editable(brushNode, face);
}

bool EditorContext::selectable(const mdl::PatchNode* patchNode) const
{
  return visible(patchNode) && editable(patchNode) && inOpenGroup(patchNode);
}

bool EditorContext::canChangeSelection() const
{
  return !m_blockSelection;
}

bool EditorContext::inOpenGroup(const mdl::Object* object) const
{
  return object->containingGroupOpened();
}

} // namespace tb::mdl
