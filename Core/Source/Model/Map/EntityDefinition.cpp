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
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include "IO/EntityDefinitionParser.h"
#include "Model/Map/Entity.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Model {
        bool compareByFlag(const SpawnFlag& left, const SpawnFlag& right) {
            return left.flag < right.flag;
        }

        bool compareByName(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2) {
            return def1->name <= def2->name;
        }
        
        bool compareByUsage(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2) {
            return def1->usageCount <= def2->usageCount;
        }
        
        EntityDefinitionPtr EntityDefinition::baseDefinition(const string& name, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_BASE;
            definition->name = name;
            definition->flags = flags;
            definition->properties = properties;
            return EntityDefinitionPtr(definition);
        }

        EntityDefinitionPtr EntityDefinition::pointDefinition(const string& name, const Vec4f& color, const BBox& bounds, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties, const string& description) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_POINT;
            definition->name = name;
            definition->color = color;
            definition->bounds = bounds;
            definition->flags = flags;
            definition->properties = properties;
            definition->description = description;
            return EntityDefinitionPtr(definition);
        }

        EntityDefinitionPtr EntityDefinition::brushDefinition(const string& name, const Vec4f& color, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties, const string& description) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_BRUSH;
            definition->name = name;
            definition->color = color;
            definition->flags = flags;
            definition->properties = properties;
            definition->description = description;
            return EntityDefinitionPtr(definition);
        }

        vector<SpawnFlag> EntityDefinition::flagsForMask(int mask) const {
            vector<SpawnFlag> result;
            map<string, SpawnFlag>::const_iterator it;
            for (it = flags.begin(); it != flags.end(); ++it)
                if ((it->second.flag & mask) != 0)
                    result.push_back(it->second);
            sort(result.begin(), result.end(), compareByFlag);
            return result;
        }

        bool EntityDefinition::flagSetOnEntity(const string& name, const Entity& entity) const {
            const string* entityFlagsStr = entity.propertyForKey(SpawnFlagsKey);
            if (entityFlagsStr == NULL)
                return false;
            map<string, SpawnFlag>::const_iterator it = flags.find(name);
            if (it == flags.end())
                return false;
            return (it->second.flag & atoi(entityFlagsStr->c_str())) != 0;

        }

        ModelPropertyPtr EntityDefinition::modelPropertyForEntity(const Entity& entity) const {
            ModelPropertyPtr defaultProperty;
            ModelPropertyPtr specificProperty;
            for (int i = 0; i < properties.size() && specificProperty == NULL; i++) {
                PropertyPtr property = properties[i];
                if (property->type == TB_EDP_MODEL) {
                    ModelPropertyPtr modelProperty = tr1::static_pointer_cast<ModelProperty>(property);
                    if (modelProperty->flagName.empty())
                        defaultProperty = modelProperty;
                    else if (flagSetOnEntity(modelProperty->flagName, entity))
                        specificProperty = modelProperty;
                }
            }

            return specificProperty.get() != NULL ? specificProperty : defaultProperty;
        }

        ModelPropertyPtr EntityDefinition::defaultModelProperty() const {
            for (int i = 0; i < properties.size(); i++) {
                PropertyPtr property = properties[i];
                if (property->type == TB_EDP_MODEL) {
                    ModelPropertyPtr modelProperty = tr1::static_pointer_cast<ModelProperty>(property);
                    if (modelProperty->flagName.empty())
                        return modelProperty;
                }
            }

            return ModelPropertyPtr();
        }

        EntityDefinitionManagerMap* EntityDefinitionManager::sharedManagers = NULL;

        EntityDefinitionManager::EntityDefinitionManager(const string& path) {
            clock_t start = clock();
            IO::EntityDefinitionParser parser(path);
            EntityDefinitionPtr definition;
            while ((definition = parser.nextDefinition()).get() != NULL) {
                m_definitions[definition->name] = definition;
                m_definitionsByName.push_back(definition);
            }

            sort(m_definitionsByName.begin(), m_definitionsByName.end(), compareByName);
            log(TB_LL_INFO, "Loaded %s in %f seconds\n", path.c_str(), (clock() - start) / CLK_TCK / 10000.0f);
        }

        EntityDefinitionManagerPtr EntityDefinitionManager::sharedManager(const string& path) {
            EntityDefinitionManagerMap::iterator it = sharedManagers->find(path);
            if (it != sharedManagers->end())
                return it->second;

            EntityDefinitionManager* instance = new EntityDefinitionManager(path);
            EntityDefinitionManagerPtr instancePtr(instance);
            (*sharedManagers)[path] = instancePtr;
            return instancePtr;
        }

        EntityDefinitionPtr EntityDefinitionManager::definition(const string& name) const {
            map<const string, EntityDefinitionPtr>::const_iterator it = m_definitions.find(name);
            if (it == m_definitions.end())
                return EntityDefinitionPtr();
            return it->second;
        }

        const vector<EntityDefinitionPtr>& EntityDefinitionManager::definitions() const {
            return m_definitionsByName;
        }

        vector<EntityDefinitionPtr> EntityDefinitionManager::definitions(EEntityDefinitionType type) const {
            return definitions(type, ES_NAME);
        }

        vector<EntityDefinitionPtr>EntityDefinitionManager::definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const {
            vector<EntityDefinitionPtr> definitionsOfType;
            for (int i = 0; i < m_definitionsByName.size(); i++)
                if (m_definitionsByName[i]->type == type)
                    definitionsOfType.push_back(m_definitionsByName[i]);
            if (criterion == ES_USAGE)
                sort(definitionsOfType.begin(), definitionsOfType.end(), compareByUsage);
            return definitionsOfType;
        }
    }
}
