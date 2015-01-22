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

namespace TrenchBroom {
    namespace Model {
        EditorContext::EditorContext() :
        m_showPointEntities(true),
        m_showBrushes(true),
        m_hiddenBrushContentTypes(0),
        m_entityLinkMode(EntityLinkMode_Direct),
        m_textureLock(false) {}
        
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

        class NodeVisible : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            const EditorContext& m_this;
        public:
            NodeVisible(const EditorContext& i_this) : m_this(i_this) {}
        private:
            void doVisit(const Model::World* world)   { setResult(true); }
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
        
        bool EditorContext::visible(const Model::Layer* layer) const {
            return !layer->hidden();
        }
        
        bool EditorContext::visible(const Model::Group* group) const {
            return true;
        }
        
        bool EditorContext::visible(const Model::Entity* entity) const {
            if (entity->selected())
                return true;
            const Model::Layer* layer = entity->layer();
            assert(layer != NULL);
            if (layer->hidden())
                return false;
            if (entity->pointEntity() && !m_showPointEntities)
                return false;
            if (entityDefinitionHidden(entity->definition()))
                return false;
            return true;
        }
        
        bool EditorContext::visible(const Model::Brush* brush) const {
            if (!m_showBrushes)
                return false;
            if (brush->hasContentType(m_hiddenBrushContentTypes))
                return false;
            const Model::Layer* layer = brush->layer();
            assert(layer != NULL);
            if (layer->hidden())
                return false;
            return visible(brush->entity());
        }
        
        bool EditorContext::visible(const Model::BrushFace* face) const {
            return visible(face->brush());
        }

        class NodeLocked : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            const EditorContext& m_this;
        public:
            NodeLocked(const EditorContext& i_this) : m_this(i_this) {}
        private:
            void doVisit(const Model::World* world)   { setResult(false); }
            void doVisit(const Model::Layer* layer)   { setResult(m_this.locked(layer)); }
            void doVisit(const Model::Group* group)   { setResult(m_this.locked(group)); }
            void doVisit(const Model::Entity* entity) { setResult(m_this.locked(entity)); }
            void doVisit(const Model::Brush* brush)   { setResult(m_this.locked(brush)); }
        };
        
        bool EditorContext::locked(const Model::Node* node) const {
            NodeLocked visitor(*this);
            node->accept(visitor);
            return visitor.result();
        }
        
        bool EditorContext::locked(const Model::Layer* layer) const {
            return layer->locked();
        }
        
        bool EditorContext::locked(const Model::Group* group) const {
            return locked(group->layer());
        }
        
        bool EditorContext::locked(const Model::Entity* entity) const {
            return locked(entity->layer());
        }
        
        bool EditorContext::locked(const Model::Brush* brush) const {
            return locked(brush->layer());
        }
        
        bool EditorContext::locked(const Model::BrushFace* face) const {
            return locked(face->brush()->layer());
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
            return visible(group);
        }
        
        bool EditorContext::pickable(const Model::Entity* entity) const {
            return visible(entity) && !entity->hasChildren();
        }
        
        bool EditorContext::pickable(const Model::Brush* brush) const {
            return visible(brush);
        }
        
        bool EditorContext::pickable(const Model::BrushFace* face) const {
            return visible(face);
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
            return pickable(group) && !locked(group);
        }
        
        bool EditorContext::selectable(const Model::Entity* entity) const {
            return pickable(entity) && !locked(entity);
        }
        
        bool EditorContext::selectable(const Model::Brush* brush) const {
            return pickable(brush) && !locked(brush);
        }

        bool EditorContext::selectable(const Model::BrushFace* face) const {
            return pickable(face) && !locked(face);
        }
    }
}
