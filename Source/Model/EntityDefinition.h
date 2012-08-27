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

#ifndef __TrenchBroom__EntityDefinition__
#define __TrenchBroom__EntityDefinition__

#include "Model/EntityDefinitionTypes.h"
#include "Model/PropertyDefinition.h"
#include "Utility/Color.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        
        class Spawnflag {
        protected:
            String m_name;
            int m_value;
        public:
            Spawnflag(const String& name, int value) : m_name(name), m_value(value) {}
            
            inline const String& name() const {
                return m_name;
            }
            
            inline int value() const {
                return m_value;
            }
        };
        
        class PointEntityModel {
        protected:
            String m_name;
            String m_flagName;
            unsigned int m_skinIndex;
        public:
            PointEntityModel(const String& name, const String& flagName, unsigned int skinIndex) : m_name(name), m_flagName(flagName), m_skinIndex(skinIndex) {}
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const String& flagName() const {
                return m_flagName;
            }
            
            inline unsigned int skinIndex() const {
                return m_skinIndex;
            }
        };
        
        class EntityDefinition {
        protected:
            String m_name;
            Color m_color;
            SpawnflagList m_spawnflags;
            String m_description;
            unsigned int m_usageCount;
            PropertyDefinition::List m_propertyDefinitions;
        public:
            enum class Type {
                Point,
                Brush
            };
            
            EntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const String& description, const PropertyDefinition::List& propertyDefinitions);
            virtual ~EntityDefinition();
            
            virtual Type type() const = 0;
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const Color& color() const {
                return m_color;
            }
            
            inline void incUsageCount() {
                m_usageCount++;
            }
            
            inline void decUsageCount() {
                m_usageCount--;
            }
            
            inline unsigned int usageCount() const {
                return m_usageCount;
            }
        };
        
        class PointEntityDefinition : public EntityDefinition {
        protected:
            BBox m_bounds;
            PointEntityModel* m_model;
        public:
            PointEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const BBox& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions);
            PointEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const BBox& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions, const PointEntityModel& model);
            ~PointEntityDefinition();
            
            inline Type type() const {
                return Type::Point;
            }
            
            inline const BBox& bounds() const {
                return m_bounds;
            }
            
            inline const PointEntityModel* model() const {
                return m_model;
            }
            
        };
        
        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const String& name, const Color& color, const SpawnflagList& spawnflags, const String& description, const PropertyDefinition::List& propertyDefinitions);
            ~BrushEntityDefinition();
            
            inline Type type() const {
                return Type::Brush;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinition__) */
