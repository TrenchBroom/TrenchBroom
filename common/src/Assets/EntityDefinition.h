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

#ifndef TrenchBroom_EntityDefinition
#define TrenchBroom_EntityDefinition

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/ModelDefinition.h"

namespace TrenchBroom {
    namespace Assets {
        class AttributeDefinition;
        class FlagsAttributeDefinition;
        
        class EntityDefinition {
        public:
            enum SortOrder {
                Name,
                Usage
            };

            typedef enum {
                Type_PointEntity,
                Type_BrushEntity
            } Type;
        private:
            size_t m_index;
            String m_name;
            Color m_color;
            String m_description;
            size_t m_usageCount;
            AttributeDefinitionList m_attributeDefinitions;
        public:
            virtual ~EntityDefinition();
            
            size_t index() const;
            void setIndex(size_t index);
            
            virtual Type type() const = 0;
            const String& name() const;
            String shortName() const;
            String groupName() const;
            const Color& color() const;
            const String& description() const;
            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();
            
            const FlagsAttributeDefinition* spawnflags() const;
            const AttributeDefinitionList& attributeDefinitions() const;
            const AttributeDefinition* attributeDefinition(const Model::AttributeName& attributeKey) const;
            
            static const AttributeDefinition* safeGetAttributeDefinition(const EntityDefinition* entityDefinition, const Model::AttributeName& attributeKey);

            static EntityDefinitionList filterAndSort(const EntityDefinitionList& definitions, EntityDefinition::Type type, SortOrder prder = Name);
        protected:
            EntityDefinition(const String& name, const Color& color, const String& description, const AttributeDefinitionList& attributeDefinitions);
        };
        
        class PointEntityDefinition : public EntityDefinition {
        private:
            BBox3 m_bounds;
            ModelDefinitionList m_modelDefinitions;
        public:
            PointEntityDefinition(const String& name, const Color& color, const BBox3& bounds, const String& description, const AttributeDefinitionList& attributeDefinitions, const ModelDefinitionList& modelDefinitions = EmptyModelDefinitionList);
            
            Type type() const;
            const BBox3& bounds() const;
            ModelSpecification model(const Model::EntityAttributes& attributes) const;
            ModelSpecification defaultModel() const;
            const ModelDefinitionList& modelDefinitions() const;
        };
    
        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const String& name, const Color& color, const String& description, const AttributeDefinitionList& attributeDefinitions);
            Type type() const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinition) */
