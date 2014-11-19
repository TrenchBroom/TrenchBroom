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

#include "ModelFilter.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Layer.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        ModelFilter::ModelFilter() :
        m_showPointEntities(true),
        m_showBrushes(true),
        m_hiddenBrushContentTypes(0),
        m_entityLinkMode(EntityLinkMode_Direct) {}
        
        ModelFilter::~ModelFilter() {}
        
        bool ModelFilter::showPointEntities() const {
            return m_showPointEntities;
        }
        
        void ModelFilter::setShowPointEntities(const bool showPointEntities) {
            if (showPointEntities == m_showPointEntities)
                return;
            m_showPointEntities = showPointEntities;
            filterDidChangeNotifier();
        }
        
        bool ModelFilter::showBrushes() const {
            return m_showBrushes;
        }
        
        void ModelFilter::setShowBrushes(const bool showBrushes) {
            if (showBrushes == m_showBrushes)
                return;
            m_showBrushes = showBrushes;
            filterDidChangeNotifier();
        }
        
        BrushContentType::FlagType ModelFilter::hiddenBrushContentTypes() const {
            return m_hiddenBrushContentTypes;
        }
        
        void ModelFilter::setHiddenBrushContentTypes(BrushContentType::FlagType brushContentTypes) {
            if (brushContentTypes == m_hiddenBrushContentTypes)
                return;
            m_hiddenBrushContentTypes = brushContentTypes;
            filterDidChangeNotifier();
        }
        
        bool ModelFilter::entityDefinitionHidden(const Assets::EntityDefinition* definition) const {
            if (definition == NULL)
                return false;
            return m_hiddenEntityDefinitions[definition->index()];
        }
        
        void ModelFilter::setEntityDefinitionHidden(const Assets::EntityDefinition* definition, const bool hidden) {
            if (definition == NULL || entityDefinitionHidden(definition) == hidden)
                return;
            m_hiddenEntityDefinitions[definition->index()] = hidden;
            filterDidChangeNotifier();
        }

        ModelFilter::EntityLinkMode ModelFilter::entityLinkMode() const {
            return m_entityLinkMode;
        }
        
        void ModelFilter::setEntityLinkMode(const EntityLinkMode entityLinkMode) {
            if (entityLinkMode == m_entityLinkMode)
                return;
            m_entityLinkMode = entityLinkMode;
            filterDidChangeNotifier();
        }
        
        struct ObjectVisible : public ConstObjectVisitor {
            const ModelFilter& filter;
            bool result;
            
            ObjectVisible(const ModelFilter& i_filter) :
            filter(i_filter),
            result(false) {}
            
            void doVisit(const Entity* entity) {
                result = entityVisible(entity);
            }
            
            void doVisit(const Brush* brush) {
                result = brushVisible(brush);
            }
            
            bool entityVisible(const Entity* entity) const {
                if (entity->worldspawn())
                    return false;
                if (entity->pointEntity() && !filter.showPointEntities())
                    return false;
                if (filter.entityDefinitionHidden(entity->definition()))
                    return false;
                return true;
            }
            
            bool brushVisible(const Brush* brush) const {
                if (!filter.showBrushes())
                    return false;
                if (brush->hasContentType(filter.hiddenBrushContentTypes()))
                    return false;
                const Entity* entity = brush->parent();
                if (filter.entityDefinitionHidden(entity->definition()))
                    return false;
                return true;
            }
        };
        
        bool ModelFilter::visible(const Object* object) const {
            if (object->selected())
                return true;
            if (!object->layer()->visible())
                return false;

            ObjectVisible objectVisible(*this);
            object->accept(objectVisible);
            return objectVisible.result;
        }
        
        bool ModelFilter::visible(const BrushFace* face) const {
            return visible(face->parent());
        }
        
        bool ModelFilter::locked(const Object* object) const {
            return object->layer()->locked();
        }

        struct ObjectPickable : public ConstObjectVisitor {
            const ModelFilter& filter;
            bool result;
            
            ObjectPickable(const ModelFilter& i_filter) :
            filter(i_filter),
            result(false) {}
            
            void doVisit(const Entity* entity) {
                result = entityPickable(entity);
            }
            
            void doVisit(const Brush* brush) {
                result = brushPickable(brush);
            }

            bool entityPickable(const Entity* entity) const {
                if (!entity->brushes().empty())
                    return false;
                return true;
            }
            
            bool brushPickable(const Brush* brush) const {
                return true;
            }
        };
        bool ModelFilter::pickable(const Object* object) const {
            if (!visible(object))
                return false;
            
            ObjectPickable objectPickable(*this);
            object->accept(objectPickable);
            return objectPickable.result;
        }
        
        bool ModelFilter::pickable(const BrushFace* face) const {
            return visible(face);
        }

        bool ModelFilter::selectable(const Object* object) const {
            return !locked(object) && pickable(object);
        }
        
        bool ModelFilter::selectable(const BrushFace* face) const {
            return !locked(face->parent()) && pickable(face);
        }
    }
}
