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

#include "Utility/List.h"

namespace TrenchBroom {
    namespace Model {
        ModelDefinitionPropertyEvaluator::ModelDefinitionPropertyEvaluator(const PropertyKey& propertyKey, const PropertyValue& propertyValue) :
        m_propertyKey(propertyKey),
        m_propertyValue(propertyValue) {}
        
        bool ModelDefinitionPropertyEvaluator::evaluate(const PropertyList& properties) const {
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Property& property = *it;
                if (property.key() == m_propertyKey) {
                    if (property.value() == m_propertyValue)
                        return true;
                    break;
                }
            }
            return false;
        }

        ModelDefinitionFlagEvaluator::ModelDefinitionFlagEvaluator(const PropertyKey& propertyKey, int flagValue) :
        m_propertyKey(propertyKey),
        m_flagValue(flagValue) {}
        
        bool ModelDefinitionFlagEvaluator::evaluate(const PropertyList& properties) const {
            PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Property& property = *it;
                if (property.key() == m_propertyKey) {
                    if ((std::atoi(property.value().c_str()) & m_flagValue) != 0)
                        return true;
                    break;
                }
            }
            return false;
        }

        ModelDefinitionPropertiesEvaluator::ModelDefinitionPropertiesEvaluator(const PropertyKey& modelKey, const PropertyKey& skinKey, const PropertyKey& frameKey) :
        m_modelKey(modelKey),
        m_skinKey(skinKey),
        m_frameKey(frameKey) {}
        
        bool ModelDefinitionPropertiesEvaluator::evaluate(const PropertyList& properties) const {
            return false;
        }

        ModelDefinition::ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex) :
        m_name(name),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex) {}
        
        ModelDefinition::ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex, const PropertyKey& propertyKey, const PropertyValue& propertyValue) :
        m_name(name),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_evaluator(new ModelDefinitionPropertyEvaluator(propertyKey, propertyValue)) {}
        
        ModelDefinition::ModelDefinition(const String& name, unsigned int skinIndex, unsigned int frameIndex, const PropertyKey& propertyKey, int flagValue) :
        m_name(name),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_evaluator(new ModelDefinitionFlagEvaluator(propertyKey, flagValue)) {}
        
        EntityDefinition::EntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinition::List& propertyDefinitions) :
        m_name(name),
        m_color(color),
        m_description(description),
        m_usageCount(0),
        m_propertyDefinitions(propertyDefinitions) {
        }
        
        PointEntityDefinition::PointEntityDefinition(const String& name, const Color& color, const BBoxf& bounds, const String& description, const PropertyDefinition::List& propertyDefinitions, const ModelDefinition::List& modelDefinitions) :
        EntityDefinition(name, color, description, propertyDefinitions),
        m_bounds(bounds),
        m_modelDefinitions(modelDefinitions) {}
        
        const ModelDefinition* PointEntityDefinition::model(const PropertyList& properties) const {
            if (m_modelDefinitions.empty())
                return NULL;
            
            ModelDefinition::List::const_reverse_iterator it, end;
            for (it = m_modelDefinitions.rbegin(), end = m_modelDefinitions.rend(); it != end; ++it) {
                const ModelDefinition::Ptr definition = *it;
                if (definition->matches(properties))
                    return definition.get();
            }
            
            return NULL;
        }

        BrushEntityDefinition::BrushEntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinition::List& propertyDefinitions) :
        EntityDefinition(name, color, description, propertyDefinitions) {}
    }
}
