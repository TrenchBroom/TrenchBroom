/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Entity.h"

#include "Model/Brush.h"
#include "Model/EntityDefinition.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        Entity::Entity(const BBox& worldBounds) :
        m_worldBounds(worldBounds),
        m_map(NULL),
        m_editState(EditState::Default),
        m_geometryValid(false) {}

        Entity::~Entity() {
            setMap(NULL);
            while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();
            m_brushes.clear();
            setDefinition(NULL);
            m_geometryValid = false;
        }

        void Entity::addBrush(Brush* brush) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            brush->setEntity(this);
            m_brushes.push_back(brush);
            invalidateGeometry();
        }
        
        void Entity::addBrushes(const BrushList& brushes) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                brush->setEntity(this);
                m_brushes.push_back(brush);
            }
            invalidateGeometry();
        }
        
        void Entity::removeBrush(Brush* brush) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            brush->setEntity(NULL);
            m_brushes.erase(std::remove(m_brushes.begin(), m_brushes.end(), brush), m_brushes.end());
            invalidateGeometry();
        }

        void Entity::setDefinition(EntityDefinition* definition) {
        }
    }
}