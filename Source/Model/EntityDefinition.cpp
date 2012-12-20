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

#include "EntityDefinition.h"

namespace TrenchBroom {
    namespace Model {
        EntityDefinition::EntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const String& description, const PropertyDefinition::List& propertyDefinitions) :
        m_name(name),
        m_color(color),
        m_spawnflags(spawnflags),
        m_description(description),
        m_usageCount(0),
        m_propertyDefinitions(propertyDefinitions) {
        }
        
        EntityDefinition::~EntityDefinition() {
        }

        PointEntityDefinition::PointEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const BBox& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions) :
        EntityDefinition(name, color, spawnflags, description, propertyDefinitions),
        m_bounds(bounds),
        m_model(NULL) {
        }
        
        PointEntityDefinition::PointEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const BBox& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions, const PointEntityModel& model) :
        EntityDefinition(name, color, spawnflags, description, propertyDefinitions),
        m_bounds(bounds),
        m_model(new PointEntityModel(model)) {
        }

        PointEntityDefinition::~PointEntityDefinition() {
            if (m_model != NULL) {
                delete m_model;
                m_model = NULL;
            }
        }

        BrushEntityDefinition::BrushEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const String& description, const PropertyDefinition::List& propertyDefinitions) :
        EntityDefinition(name, color, spawnflags, description, propertyDefinitions) {
        }
        
        BrushEntityDefinition::~BrushEntityDefinition() {
        }
    }
}
