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

#include "EditorContext.h"

#include "Ensure.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"

namespace TrenchBroom {
    namespace Model {
        EditorContext::EditorContext() {
            reset();
        }

        void EditorContext::reset() {
            m_hiddenTags = 0;
            m_hiddenEntityDefinitions.reset();
            m_blockSelection = false;
            m_currentGroup = nullptr;
        }

        TagType::Type EditorContext::hiddenTags() const {
            return m_hiddenTags;
        }

        void EditorContext::setHiddenTags(const TagType::Type hiddenTags) {
            if (hiddenTags != m_hiddenTags) {
                m_hiddenTags = hiddenTags;
                editorContextDidChangeNotifier();
            }
        }

        bool EditorContext::entityDefinitionHidden(const Model::AttributableNode* entityNode) const {
            return entityNode != nullptr && entityDefinitionHidden(entityNode->entity().definition());
        }

        bool EditorContext::entityDefinitionHidden(const Assets::EntityDefinition* definition) const {
            return definition != nullptr && m_hiddenEntityDefinitions[definition->index()];
        }

        void EditorContext::setEntityDefinitionHidden(const Assets::EntityDefinition* definition, const bool hidden) {
            if (definition != nullptr && entityDefinitionHidden(definition) != hidden) {
                m_hiddenEntityDefinitions[definition->index()] = hidden;
                editorContextDidChangeNotifier();
            }
        }

        bool EditorContext::blockSelection() const {
            return m_blockSelection;
        }

        void EditorContext::setBlockSelection(const bool blockSelection) {
            if (m_blockSelection != blockSelection) {
                m_blockSelection = blockSelection;
                editorContextDidChangeNotifier();
            }
        }

        Model::GroupNode* EditorContext::currentGroup() const {
            return m_currentGroup;
        }

        void EditorContext::pushGroup(Model::GroupNode* groupNode) {
            ensure(groupNode != nullptr, "group is null");
            assert(m_currentGroup == nullptr || groupNode->group() == m_currentGroup);

            if (m_currentGroup != nullptr) {
                m_currentGroup->close();
            }
            m_currentGroup = groupNode;
            m_currentGroup->open();
        }

        void EditorContext::popGroup() {
            ensure(m_currentGroup != nullptr, "currentGroup is null");
            m_currentGroup->close();
            m_currentGroup = m_currentGroup->group();
            if (m_currentGroup != nullptr) {
                m_currentGroup->open();
            }
        }

        bool EditorContext::visible(const Model::Node* node) const {
            return node->accept(kdl::overload(
                [&](const WorldNode* world)   { return visible(world); },
                [&](const LayerNode* layer)   { return visible(layer); },
                [&](const GroupNode* group)   { return visible(group); },
                [&](const EntityNode* entity) { return visible(entity); },
                [&](const BrushNode* brush)   { return visible(brush); }
            ));
        }

        bool EditorContext::visible(const Model::WorldNode* worldNode) const {
            return worldNode->visible();
        }

        bool EditorContext::visible(const Model::LayerNode* layerNode) const {
            return layerNode->visible();
        }

        bool EditorContext::visible(const Model::GroupNode* groupNode) const {
            if (groupNode->selected()) {
                return true;
            }
            if (!anyChildVisible(groupNode)) {
                return false;
            }
            return groupNode->visible();
        }

        bool EditorContext::visible(const Model::EntityNode* entityNode) const {
            if (entityNode->selected()) {
                return true;
            }

            if (!entityNode->entity().pointEntity()) {
                if (!anyChildVisible(entityNode)) {
                    return false;
                }
                return true;
            }

            if (!entityNode->visible()) {
                return false;
            }

            if (entityNode->entity().pointEntity() && !pref(Preferences::ShowPointEntities)) {
                return false;
            }

            if (entityDefinitionHidden(entityNode)) {
                return false;
            }

            return true;
        }

        bool EditorContext::visible(const Model::BrushNode* brushNode) const {
            if (brushNode->selected()) {
                return true;
            }

            if (!pref(Preferences::ShowBrushes)) {
                return false;
            }

            if (brushNode->hasTag(m_hiddenTags)) {
                return false;
            }

            if (brushNode->allFacesHaveAnyTagInMask(m_hiddenTags)) {
                return false;
            }

            if (entityDefinitionHidden(brushNode->entity())) {
                return false;
            }

            return brushNode->visible();
        }

        bool EditorContext::visible(const Model::BrushNode* brushNode, const Model::BrushFace& face) const {
            return visible(brushNode) && !face.hasTag(m_hiddenTags);
        }

        bool EditorContext::anyChildVisible(const Model::Node* node) const {
            const auto& children = node->children();
            return std::any_of(std::begin(children), std::end(children), [this](const Node* child) { return visible(child); });
        }

        bool EditorContext::editable(const Model::Node* node) const {
            return node->editable();
        }

        bool EditorContext::editable(const Model::BrushNode* brushNode, const Model::BrushFace&) const {
            return editable(brushNode);
        }

        bool EditorContext::pickable(const Model::Node* node) const {
            return node->accept(kdl::overload(
                [&](const WorldNode* world)   { return pickable(world); },
                [&](const LayerNode* layer)   { return pickable(layer); },
                [&](const GroupNode* group)   { return pickable(group); },
                [&](const EntityNode* entity) { return pickable(entity); },
                [&](const BrushNode* brush)   { return pickable(brush); }
            ));
        }

        bool EditorContext::pickable(const Model::WorldNode*) const {
            return false;
        }

        bool EditorContext::pickable(const Model::LayerNode*) const {
            return false;
        }

        bool EditorContext::pickable(const Model::GroupNode* groupNode) const {
            return visible(groupNode) && !groupNode->opened() && groupNode->groupOpened();
        }

        bool EditorContext::pickable(const Model::EntityNode* entityNode) const {
            // Do not check whether this is an open group or not -- we must be able
            // to pick objects within groups in order to draw on them etc.
            return visible(entityNode) && !entityNode->hasChildren();
        }

        bool EditorContext::pickable(const Model::BrushNode* brushNode) const {
            // Do not check whether this is an open group or not -- we must be able
            // to pick objects within groups in order to draw on them etc.
            return visible(brushNode);
        }

        bool EditorContext::pickable(const Model::BrushNode* brushNode, const Model::BrushFace& face) const {
            return brushNode->selected() || visible(brushNode, face);
        }

        bool EditorContext::selectable(const Model::Node* node) const {
            return node->accept(kdl::overload(
                [&](const WorldNode* world)   { return selectable(world); },
                [&](const LayerNode* layer)   { return selectable(layer); },
                [&](const GroupNode* group)   { return selectable(group); },
                [&](const EntityNode* entity) { return selectable(entity); },
                [&](const BrushNode* brush)   { return selectable(brush); }
            ));
        }

        bool EditorContext::selectable(const Model::WorldNode*) const {
            return false;
        }

        bool EditorContext::selectable(const Model::LayerNode*) const {
            return false;
        }

        bool EditorContext::selectable(const Model::GroupNode* groupNode) const {
            return visible(groupNode) && editable(groupNode) && pickable(groupNode) && inOpenGroup(groupNode);
        }

        bool EditorContext::selectable(const Model::EntityNode* entityNode) const {
            return visible(entityNode) && editable(entityNode) && pickable(entityNode) && inOpenGroup(entityNode);
        }

        bool EditorContext::selectable(const Model::BrushNode* brushNode) const {
            return visible(brushNode) && editable(brushNode) && pickable(brushNode) && inOpenGroup(brushNode);
        }

        bool EditorContext::selectable(const Model::BrushNode* brushNode, const Model::BrushFace& face) const {
            return visible(brushNode, face) && editable(brushNode, face) && pickable(brushNode, face);
        }

        bool EditorContext::canChangeSelection() const {
            return !m_blockSelection;
        }

        bool EditorContext::inOpenGroup(const Model::Object* object) const {
            return object->groupOpened();
        }
    }
}
