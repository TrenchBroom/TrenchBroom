/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Model/EntityProperties.h"
#include "Assets/ModelDefinition.h"

namespace TrenchBroom {
    namespace Assets {
        class PropertyDefinition;
        class FlagsPropertyDefinition;
        
        class EntityDefinition {
        public:
            typedef enum {
                PointEntity,
                BrushEntity
            } Type;
        private:
            String m_name;
            Color m_color;
            String m_description;
            size_t m_usageCount;
            PropertyDefinitionList m_propertyDefinitions;
        public:
            virtual ~EntityDefinition();
            
            virtual Type type() const = 0;
            const String& name() const;
            String shortName() const;
            String groupName() const;
            const Color& color() const;
            const String& description() const;
            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();
            
            const FlagsPropertyDefinition* spawnflags() const;
            const PropertyDefinitionList& propertyDefinitions() const;
            const PropertyDefinition* propertyDefinition(const Model::PropertyKey& propertyKey) const;
        protected:
            EntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinitionList& propertyDefinitions);
        };
        
        class PointEntityDefinition : public EntityDefinition {
        private:
            BBox3 m_bounds;
            ModelDefinitionList m_modelDefinitions;
        public:
            PointEntityDefinition(const String& name, const Color& color, const BBox3& bounds, const String& description, const PropertyDefinitionList propertyDefinitions, const ModelDefinitionList& modelDefinitions = EmptyModelDefinitionList);
            
            Type type() const;
            const BBox3& bounds() const;
            ModelSpecification model(const Model::EntityProperties& properties) const;
            const ModelDefinitionList& modelDefinitions() const;
        };
    
        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinitionList& propertyDefinitions);
            Type type() const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinition__) */
