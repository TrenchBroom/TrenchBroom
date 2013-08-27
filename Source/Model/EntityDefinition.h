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

#include "Model/Entity.h"
#include "Model/EntityDefinitionTypes.h"
#include "Model/EntityProperty.h"
#include "Model/PropertyDefinition.h"
#include "Utility/Color.h"
#include "Utility/SharedPointer.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <cstdlib>
#include <vector>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class ModelDefinitionEvaluator {
        public:
            typedef std::tr1::shared_ptr<ModelDefinitionEvaluator> Ptr;
            
            virtual ~ModelDefinitionEvaluator() {}
            
            virtual bool evaluate(const PropertyList& properties) const = 0;
        };
        
        class ModelDefinitionPropertyEvaluator : public ModelDefinitionEvaluator {
        private:
            PropertyKey m_propertyKey;
            PropertyValue m_propertyValue;
        public:
            ModelDefinitionPropertyEvaluator(const PropertyKey& propertyKey, const PropertyValue& propertyValue);
            
            bool evaluate(const PropertyList& properties) const;
        };
        
        class ModelDefinitionFlagEvaluator : public ModelDefinitionEvaluator {
        private:
            PropertyKey m_propertyKey;
            int m_flagValue;
        public:
            ModelDefinitionFlagEvaluator(const PropertyKey& propertyKey, int flagValue);
            
            bool evaluate(const PropertyList& properties) const;
        };
        
        class ModelDefinitionPropertiesEvaluator : public ModelDefinitionEvaluator {
        private:
            PropertyKey m_modelKey;
            PropertyKey m_skinKey;
            PropertyKey m_frameKey;
        public:
            ModelDefinitionPropertiesEvaluator(const PropertyKey& modelKey, const PropertyKey& skinKey, const PropertyKey& frameKey);
            
            bool evaluate(const PropertyList& properties) const;
        };
        
        class ModelDefinition {
        public:
            typedef std::tr1::shared_ptr<ModelDefinition> Ptr;
            typedef std::vector<Ptr> List;
        private:
            String m_name;
            unsigned int m_skinIndex;
            unsigned int m_frameIndex;
        
            ModelDefinitionEvaluator::Ptr m_evaluator;
        public:
            ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex);
            ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex, const PropertyKey& propertyKey, const PropertyValue& propertyValue);
            ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex, const PropertyKey& propertyKey, int flagValue);
            
            inline const String& name() const {
                return m_name;
            }
            
            inline unsigned int skinIndex() const {
                return m_skinIndex;
            }
            
            inline unsigned int frameIndex() const {
                return m_frameIndex;
            }
            
            inline bool matches(const PropertyList& properties) const {
                if (m_evaluator == NULL)
                    return true;
                return m_evaluator->evaluate(properties);
            }
        };
        
        class EntityDefinition {
        protected:
            String m_name;
            Color m_color;
            String m_description;
            unsigned int m_usageCount;
            PropertyDefinition::List m_propertyDefinitions;
        public:
            enum Type {
                PointEntity,
                BrushEntity
            };
            
            EntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinition::List& propertyDefinitions);
            virtual ~EntityDefinition() {}
            
            virtual Type type() const = 0;
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const String groupName() const {
                size_t uscoreIndex = m_name.find_first_of('_');
                if (uscoreIndex == String::npos)
                    return "";
                return Utility::capitalize(m_name.substr(0, uscoreIndex));
            }
            
            inline const String shortName() const {
                size_t uscoreIndex = m_name.find_first_of('_');
                if (uscoreIndex == String::npos)
                    return m_name;
                return m_name.substr(uscoreIndex + 1);
            }
            
            inline const Color& color() const {
                return m_color;
            }
            
            const FlagsPropertyDefinition* spawnflags() const {
                PropertyDefinition::List::const_iterator it, end;
                for (it = m_propertyDefinitions.begin(), end = m_propertyDefinitions.end(); it != end; ++it) {
                    const PropertyDefinition::Ptr definition = *it;
                    if (definition->type() == PropertyDefinition::FlagsProperty &&
                        definition->name() == Model::Entity::SpawnFlagsKey)
                        return static_cast<const FlagsPropertyDefinition*>(definition.get());
                }
                return NULL;
            }
            
            const PropertyDefinition* propertyDefinition(const PropertyKey& propertyKey) const {
                PropertyDefinition::List::const_iterator it, end;
                for (it = m_propertyDefinitions.begin(), end = m_propertyDefinitions.end(); it != end; ++it) {
                    const PropertyDefinition::Ptr definition = *it;
                    if (definition->name() == propertyKey)
                        return definition.get();
                }
                return NULL;
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
            BBoxf m_bounds;
            ModelDefinition::List m_modelDefinitions;
        public:
            PointEntityDefinition(const String& name, const Color& color, const BBoxf& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions, const ModelDefinition::List& modelDefinitions = ModelDefinition::List());
            
            inline Type type() const {
                return PointEntity;
            }
            
            inline const BBoxf& bounds() const {
                return m_bounds;
            }

            const ModelDefinition* model(const PropertyList& properties = EmptyPropertyList) const;
        };
        
        class BrushEntityDefinition : public EntityDefinition {
        public:
            BrushEntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinition::List& propertyDefinitions);
            
            inline Type type() const {
                return BrushEntity;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinition__) */
