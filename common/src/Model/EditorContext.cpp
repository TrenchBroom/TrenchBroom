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

        bool EditorContext::entityDefinitionHidden(const Model::AttributableNode* entity) const {
            return entity != nullptr && entityDefinitionHidden(entity->definition());
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

        void EditorContext::pushGroup(Model::GroupNode* group) {
            ensure(group != nullptr, "group is null");
            assert(m_currentGroup == nullptr || group->group() == m_currentGroup);

            if (m_currentGroup != nullptr) {
                m_currentGroup->close();
            }
            m_currentGroup = group;
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

        bool EditorContext::visible(const Model::WorldNode* world) const {
            return world->visible();
        }

        bool EditorContext::visible(const Model::LayerNode* layer) const {
            return layer->visible();
        }

        bool EditorContext::visible(const Model::GroupNode* group) const {
            if (group->selected()) {
                return true;
            }
            if (!anyChildVisible(group)) {
                return false;
            }
            return group->visible();
        }

        bool EditorContext::visible(const Model::EntityNode* entity) const {
            if (entity->selected()) {
                return true;
            }

            if (entity->brushEntity()) {
                if (!anyChildVisible(entity)) {
                    return false;
                }
                return true;
            }

            if (!entity->visible()) {
                return false;
            }

            if (entity->pointEntity() && !pref(Preferences::ShowPointEntities)) {
                return false;
            }

            if (entityDefinitionHidden(entity)) {
                return false;
            }

            return true;
        }

        bool EditorContext::visible(const Model::BrushNode* brush) const {
            if (brush->selected()) {
                return true;
            }

            if (!pref(Preferences::ShowBrushes)) {
                return false;
            }

            if (brush->hasTag(m_hiddenTags)) {
                return false;
            }

            if (brush->allFacesHaveAnyTagInMask(m_hiddenTags)) {
                return false;
            }

            if (entityDefinitionHidden(brush->entity())) {
                return false;
            }

            return brush->visible();
        }

        bool EditorContext::visible(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            return visible(brush) && !face.hasTag(m_hiddenTags);
        }

        bool EditorContext::anyChildVisible(const Model::Node* node) const {
            const auto& children = node->children();
            return std::any_of(std::begin(children), std::end(children), [this](const Node* child) { return visible(child); });
        }

        bool EditorContext::editable(const Model::Node* node) const {
            return node->editable();
        }

        bool EditorContext::editable(const Model::BrushNode* brush, const Model::BrushFace&) const {
            return editable(brush);
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

        bool EditorContext::pickable(const Model::WorldNode* /* world */) const {
            return false;
        }

        bool EditorContext::pickable(const Model::LayerNode* /* layer */) const {
            return false;
        }

        bool EditorContext::pickable(const Model::GroupNode* group) const {
            return visible(group) && !group->opened() && group->groupOpened();
        }

        bool EditorContext::pickable(const Model::EntityNode* entity) const {
            // Do not check whether this is an open group or not -- we must be able
            // to pick objects within groups in order to draw on them etc.
            return visible(entity) && !entity->hasChildren();
        }

        bool EditorContext::pickable(const Model::BrushNode* brush) const {
            // Do not check whether this is an open group or not -- we must be able
            // to pick objects within groups in order to draw on them etc.
            return visible(brush);
        }

        bool EditorContext::pickable(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            return brush->selected() || visible(brush, face);
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

        bool EditorContext::selectable(const Model::GroupNode* group) const {
            return visible(group) && editable(group) && pickable(group) && inOpenGroup(group);
        }

        bool EditorContext::selectable(const Model::EntityNode* entity) const {
            return visible(entity) && editable(entity) && pickable(entity) && inOpenGroup(entity);
        }

        bool EditorContext::selectable(const Model::BrushNode* brush) const {
            return visible(brush) && editable(brush) && pickable(brush) && inOpenGroup(brush);
        }

        bool EditorContext::selectable(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            return visible(brush, face) && editable(brush, face) && pickable(brush, face);
        }

        bool EditorContext::canChangeSelection() const {
            return !m_blockSelection;
        }

        bool EditorContext::inOpenGroup(const Model::Object* object) const {
            return object->groupOpened();
        }
    }
}
