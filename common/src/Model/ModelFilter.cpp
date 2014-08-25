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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        ModelFilter::ModelFilter() :
        m_showPointEntities(true),
        m_showBrushes(true),
        m_hiddenBrushContentTypes(0) {}
        
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

        bool ModelFilter::visible(const Object* object) const {
            if (object->type() == Object::Type_Entity) {
                const Entity* entity = static_cast<const Entity*>(object);
                if (entity->worldspawn())
                    return false;
                if (entity->pointEntity() && !m_showPointEntities)
                    return false;
            } else if (object->type() == Object::Type_Brush) {
                if (!m_showBrushes)
                    return false;
                const Brush* brush = static_cast<const Brush*>(object);
                if (brush->hasContentType(m_hiddenBrushContentTypes))
                    return false;
            }
            return true;
        }
        
        bool ModelFilter::visible(const BrushFace* face) const {
            return visible(face->parent());
        }
        
        bool ModelFilter::pickable(const Object* object) const {
            if (!visible(object))
                return false;
            
            if (object->type() == Object::Type_Entity) {
                const Entity* entity = static_cast<const Entity*>(object);
                if (!entity->brushes().empty())
                    return false;
                if (entity->pointEntity() && !m_showPointEntities)
                    return false;
            } else if (object->type() == Object::Type_Brush) {
                if (!m_showBrushes)
                    return false;
            }
            return true;
        }
        
        bool ModelFilter::pickable(const BrushFace* face) const {
            return visible(face);
        }

        bool ModelFilter::selectable(const Object* object) const {
            if (!pickable(object))
                return false;
            
            if (object->type() == Object::Type_Entity) {
                const Entity* entity = static_cast<const Entity*>(object);
                if (!entity->brushes().empty())
                    return false;
            }
            return true;
        }
        
        bool ModelFilter::selectable(const BrushFace* face) const {
            return pickable(face);
        }
    }
}
