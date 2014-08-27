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

#include "EntityDefinition.h"

#include "CollectionUtils.h"
#include "Assets/PropertyDefinition.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        class CompareByName {
        private:
            bool m_shortName;
        public:
            CompareByName(const bool shortName) :
            m_shortName(shortName) {}
            bool operator() (const EntityDefinition* left, const EntityDefinition* right) const {
                if (m_shortName)
                    return left->shortName() < right->shortName();
                return left->name() < right->name();
            }
        };
        
        class CompareByUsage {
        public:
            bool operator() (const EntityDefinition* left, const EntityDefinition* right) const {
                if (left->usageCount() == right->usageCount())
                    return left->name() < right->name();
                return left->usageCount() > right->usageCount();
            }
        };

        EntityDefinition::~EntityDefinition() {}
        
        size_t EntityDefinition::index() const {
            return m_index;
        }
        
        void EntityDefinition::setIndex(const size_t index) {
            m_index = index;
        }

        const String& EntityDefinition::name() const {
            return m_name;
        }
        
        String EntityDefinition::shortName() const {
            const size_t index = m_name.find_first_of('_');
            if (index == String::npos)
                return m_name;
            return m_name.substr(index+1);
        }
        
        String EntityDefinition::groupName() const {
            const size_t index = m_name.find_first_of('_');
            if (index == String::npos)
                return m_name;
            return m_name.substr(0, index);
        }
        
        const Color& EntityDefinition::color() const {
            return m_color;
        }
        
        const String& EntityDefinition::description() const {
            return m_description;
        }
        
        size_t EntityDefinition::usageCount() const {
            return m_usageCount;
        }
        
        void EntityDefinition::incUsageCount() {
            ++m_usageCount;
        }
        
        void EntityDefinition::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
        }
        
        struct FindSpawnflagsDefinition {
            bool operator()(const PropertyDefinitionPtr propertyDefinition) const {
                return (propertyDefinition->type() == PropertyDefinition::Type_FlagsProperty &&
                        propertyDefinition->name() == Model::PropertyKeys::Spawnflags);
            }
        };

        const FlagsPropertyDefinition* EntityDefinition::spawnflags() const {
            return static_cast<FlagsPropertyDefinition*>(VectorUtils::findIf(m_propertyDefinitions, FindSpawnflagsDefinition()).get());
        }
        
        struct FindPropertyDefinitionByName {
            String name;
            FindPropertyDefinitionByName(const String& i_name) : name(i_name) {}
            
            bool operator()(const PropertyDefinitionPtr propertyDefinition) const {
                return propertyDefinition->name() == name;
            }
        };

        const PropertyDefinitionList& EntityDefinition::propertyDefinitions() const {
            return m_propertyDefinitions;
        }

        const PropertyDefinition* EntityDefinition::propertyDefinition(const Model::PropertyKey& propertyKey) const {
            return VectorUtils::findIf(m_propertyDefinitions, FindPropertyDefinitionByName(propertyKey)).get();
        }

        const PropertyDefinition* EntityDefinition::safeGetPropertyDefinition(const EntityDefinition* entityDefinition, const Model::PropertyKey& propertyKey) {
            return entityDefinition != NULL ? entityDefinition->propertyDefinition(propertyKey) : NULL;
        }

        EntityDefinitionList EntityDefinition::filterAndSort(const EntityDefinitionList& definitions, const EntityDefinition::Type type, const SortOrder order) {
            EntityDefinitionList result;
            EntityDefinitionList::const_iterator it, end;
            for (it = definitions.begin(), end = definitions.end(); it != end; ++it) {
                EntityDefinition* definition = *it;
                if (definition->type() == type)
                    result.push_back(definition);
            }
            if (order == Usage)
                std::sort(result.begin(), result.end(), CompareByUsage());
            else
                std::sort(result.begin(), result.end(), CompareByName(false));
            return result;
        }

        EntityDefinition::EntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinitionList& propertyDefinitions) :
        m_index(0),
        m_name(name),
        m_color(color),
        m_description(description),
        m_usageCount(0),
        m_propertyDefinitions(propertyDefinitions) {}

        PointEntityDefinition::PointEntityDefinition(const String& name, const Color& color, const BBox3& bounds, const String& description, const PropertyDefinitionList propertyDefinitions, const ModelDefinitionList& modelDefinitions) :
        EntityDefinition(name, color, description, propertyDefinitions),
        m_bounds(bounds),
        m_modelDefinitions(modelDefinitions) {}
        
        EntityDefinition::Type PointEntityDefinition::type() const {
            return Type_PointEntity;
        }
        
        const BBox3& PointEntityDefinition::bounds() const {
            return m_bounds;
        }
        
        ModelSpecification PointEntityDefinition::model(const Model::EntityProperties& properties) const {
            ModelDefinitionList::const_reverse_iterator it, end;
            for (it = m_modelDefinitions.rbegin(), end = m_modelDefinitions.rend(); it != end; ++it) {
                ModelDefinitionPtr modelDefinition = *it;
                if (modelDefinition->matches(properties))
                    return modelDefinition->modelSpecification(properties);
            }
            return ModelSpecification(IO::Path(""), 0, 0);
        }

        ModelSpecification PointEntityDefinition::defaultModel() const {
            if (m_modelDefinitions.empty())
                return ModelSpecification(IO::Path(""), 0, 0);
            return m_modelDefinitions.front()->defaultModelSpecification();
        }

        const ModelDefinitionList& PointEntityDefinition::modelDefinitions() const {
            return m_modelDefinitions;
        }

        BrushEntityDefinition::BrushEntityDefinition(const String& name, const Color& color, const String& description, const PropertyDefinitionList& propertyDefinitions) :
        EntityDefinition(name, color, description, propertyDefinitions) {}
        
        EntityDefinition::Type BrushEntityDefinition::type() const {
            return Type_BrushEntity;
        }
    }
}
