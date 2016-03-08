/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        EditorContext::EditorContext() :
        m_showPointEntities(true),
        m_showBrushes(true),
        m_hiddenBrushContentTypes(0),
        m_entityLinkMode(EntityLinkMode_Direct),
        m_textureLock(false),
        m_blockSelection(false),
        m_currentGroup(NULL) {}
        
        bool EditorContext::showPointEntities() const {
            return m_showPointEntities;
        }
        
        void EditorContext::setShowPointEntities(const bool showPointEntities) {
            if (showPointEntities == m_showPointEntities)
                return;
            m_showPointEntities = showPointEntities;
            editorContextDidChangeNotifier();
        }
        
        bool EditorContext::showBrushes() const {
            return m_showBrushes;
        }
        
        void EditorContext::setShowBrushes(const bool showBrushes) {
            if (showBrushes == m_showBrushes)
                return;
            m_showBrushes = showBrushes;
            editorContextDidChangeNotifier();
        }
        
        Model::BrushContentType::FlagType EditorContext::hiddenBrushContentTypes() const {
            return m_hiddenBrushContentTypes;
        }
        
        void EditorContext::setHiddenBrushContentTypes(Model::BrushContentType::FlagType brushContentTypes) {
            if (brushContentTypes == m_hiddenBrushContentTypes)
                return;
            m_hiddenBrushContentTypes = brushContentTypes;
            editorContextDidChangeNotifier();
        }
        
        bool EditorContext::entityDefinitionHidden(const Model::AttributableNode* entity) const {
            if (entity == NULL)
                return false;
            return entityDefinitionHidden(entity->definition());
        }

        bool EditorContext::entityDefinitionHidden(const Assets::EntityDefinition* definition) const {
            if (definition == NULL)
                return false;
            return m_hiddenEntityDefinitions[definition->index()];
        }
        
        void EditorContext::setEntityDefinitionHidden(const Assets::EntityDefinition* definition, const bool hidden) {
            if (definition == NULL || entityDefinitionHidden(definition) == hidden)
                return;
            m_hiddenEntityDefinitions[definition->index()] = hidden;
            editorContextDidChangeNotifier();
        }
        
        EditorContext::EntityLinkMode EditorContext::entityLinkMode() const {
            return m_entityLinkMode;
        }
        
        void EditorContext::setEntityLinkMode(const EntityLinkMode entityLinkMode) {
            if (entityLinkMode == m_entityLinkMode)
                return;
            m_entityLinkMode = entityLinkMode;
            editorContextDidChangeNotifier();
        }

        bool EditorContext::textureLock() const {
            return m_textureLock;
        }
        
        void EditorContext::setTextureLock(const bool textureLock) {
            m_textureLock = textureLock;
            editorContextDidChangeNotifier();
        }

        bool EditorContext::blockSelection() const {
            return m_blockSelection;
        }
        
        void EditorContext::setBlockSelection(const bool blockSelection) {
            m_blockSelection = blockSelection;
            editorContextDidChangeNotifier();
        }

        Model::Group* EditorContext::currentGroup() const {
            return m_currentGroup;
        }

        void EditorContext::pushGroup(Model::Group* group) {
            assert(group != NULL);
            assert(m_currentGroup == NULL || group->group() == m_currentGroup);
            
            if (m_currentGroup != NULL)
                m_currentGroup->close();
            m_currentGroup = group;
            m_currentGroup->open();
        }
        
        void EditorContext::popGroup() {
            assert(m_currentGroup != NULL);
            m_currentGroup->close();
            m_currentGroup = m_currentGroup->group();
            if (m_currentGroup != NULL)
                m_currentGroup->open();
        }
        
        class NodeVisible : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            const EditorContext& m_this;
        public:
            NodeVisible(const EditorContext& i_this) : m_this(i_this) {}
        private:
            void doVisit(const Model::World* world)   { setResult(m_this.visible(world)); }
            void doVisit(const Model::Layer* layer)   { setResult(m_this.visible(layer)); }
            void doVisit(const Model::Group* group)   { setResult(m_this.visible(group)); }
            void doVisit(const Model::Entity* entity) { setResult(m_this.visible(entity)); }
            void doVisit(const Model::Brush* brush)   { setResult(m_this.visible(brush)); }
        };
        
        bool EditorContext::visible(const Model::Node* node) const {
            NodeVisible visitor(*this);
            node->accept(visitor);
            return visitor.result();
        }
        
        bool EditorContext::visible(const Model::World* world) const {
            return world->visible();
        }

        bool EditorContext::visible(const Model::Layer* layer) const {
            return layer->visible();
        }
        
        bool EditorContext::visible(const Model::Group* group) const {
            if (group->selected())
                return true;
            return group->visible();
        }
        
        bool EditorContext::visible(const Model::Entity* entity) const {
            if (entity->selected())
                return true;
            if (!entity->visible())
                return false;
            if (entity->pointEntity() && !m_showPointEntities)
                return false;
            if (entityDefinitionHidden(entity))
                return false;
            return true;
        }
        
        bool EditorContext::visible(const Model::Brush* brush) const {
            if (brush->selected())
                return true;
            if (!m_showBrushes)
                return false;
            if (brush->hasContentType(m_hiddenBrushContentTypes))
                return false;
            if (entityDefinitionHidden(brush->entity()))
                return false;
            return brush->visible();
        }
        
        bool EditorContext::visible(const Model::BrushFace* face) const {
            return visible(face->brush());
        }

        bool EditorContext::editable(const Model::Node* node) const {
            return node->editable();
        }
        
        bool EditorContext::editable(const Model::BrushFace* face) const {
            return editable(face->brush());
        }

        class NodePickable : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            const EditorContext& m_this;
        public:
            NodePickable(const EditorContext& i_this) : m_this(i_this) {}
        private:
            void doVisit(const Model::World* world)   { setResult(false); }
            void doVisit(const Model::Layer* layer)   { setResult(m_this.pickable(layer)); }
            void doVisit(const Model::Group* group)   { setResult(m_this.pickable(group)); }
            void doVisit(const Model::Entity* entity) { setResult(m_this.pickable(entity)); }
            void doVisit(const Model::Brush* brush)   { setResult(m_this.pickable(brush)); }
        };
        
        bool EditorContext::pickable(const Model::Node* node) const {
            NodePickable visitor(*this);
            node->accept(visitor);
            return visitor.result();
        }
        
        bool EditorContext::pickable(const Model::Layer* layer) const {
            return false;
        }
        
        bool EditorContext::pickable(const Model::Group* group) const {
            // Removed visible check and hardwired that into HitQuery. Invisible objects should not be considered during picking, ever.
            
            Model::Group* containingGroup = group->group();
            return (containingGroup == NULL || containingGroup->opened()) /* && visible(group) */;
        }
        
        bool EditorContext::pickable(const Model::Entity* entity) const {
            // Removed the group check to allow brushes to be picked if they are within groups.
            // This is necessary so that it is possible to draw new brushes onto grouped brushes.
            // Might break other things though.
            
            // Removed visible check and hardwired that into HitQuery. Invisible objects should not be considered during picking, ever.

            // Model::Group* containingGroup = entity->group();
            return /*(containingGroup == NULL || containingGroup->opened()) &&*/ !entity->hasChildren() /* && visible(entity) */;
        }
        
        bool EditorContext::pickable(const Model::Brush* brush) const {
            // Removed the group check to allow brushes to be picked if they are within groups.
            // This is necessary so that it is possible to draw new brushes onto grouped brushes.
            // Might break other things though.
            
            // Removed visible check and hardwired that into HitQuery. Invisible objects should not be considered during picking, ever.

            // Model::Group* containingGroup = brush->group();
            return /*(containingGroup == NULL || containingGroup->opened()) && visible(brush) */ true;
        }
        
        bool EditorContext::pickable(const Model::BrushFace* face) const {
            // Removed visible check and hardwired that into HitQuery. Invisible objects should not be considered during picking, ever.
            return /* visible(face) */ true;
        }

        class NodeSelectable : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            const EditorContext& m_this;
        public:
            NodeSelectable(const EditorContext& i_this) : m_this(i_this) {}
        private:
            void doVisit(const Model::World* world)   { setResult(false); }
            void doVisit(const Model::Layer* layer)   { setResult(m_this.selectable(layer)); }
            void doVisit(const Model::Group* group)   { setResult(m_this.selectable(group)); }
            void doVisit(const Model::Entity* entity) { setResult(m_this.selectable(entity)); }
            void doVisit(const Model::Brush* brush)   { setResult(m_this.selectable(brush)); }
        };

        bool EditorContext::selectable(const Model::Node* node) const {
            NodeSelectable visitor(*this);
            node->accept(visitor);
            return visitor.result();
        }
        
        bool EditorContext::selectable(const Model::Layer* layer) const {
            return false;
        }
        
        bool EditorContext::selectable(const Model::Group* group) const {
            return visible(group) && editable(group) && pickable(group);
        }
        
        bool EditorContext::selectable(const Model::Entity* entity) const {
            return visible(entity) && editable(entity) && pickable(entity);
        }
        
        bool EditorContext::selectable(const Model::Brush* brush) const {
            return visible(brush) && editable(brush) && pickable(brush);
        }

        bool EditorContext::selectable(const Model::BrushFace* face) const {
            return visible(face) && editable(face) && pickable(face);
        }

        bool EditorContext::canChangeSelection() const {
            return !m_blockSelection;
        }
    }
}
