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

namespace TrenchBroom {
    namespace Model {
        bool compareByName(const EntityDefinition* def1, const EntityDefinition* def2) {
            return def1->name <= def2->name;
        }

        bool compareByUsage(const EntityDefinition* def1, const EntityDefinition* def2) {
            return def1->usageCount <= def2->usageCount;
        }

        bool compareByFlag(const SpawnFlag& left, const SpawnFlag& right) {
            return left.flag < right.flag;
        }

        EntityDefinition* EntityDefinition::baseDefinition(const string& name, const map<string, SpawnFlag>& flags, const vector<Property*>& properties) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_BASE;
            definition->name = name;
            definition->flags = flags;
            definition->properties = properties;
            return definition;
        }

        EntityDefinition* EntityDefinition::pointDefinition(const string& name, const Vec4f& color, const BBox& bounds, const map<string, SpawnFlag>& flags, const vector<Property*>& properties, const string& description) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_POINT;
            definition->name = name;
            definition->color = color;
            definition->bounds = bounds;
            definition->flags = flags;
            definition->properties = properties;
            definition->description = description;
            return definition;
        }

        EntityDefinition* EntityDefinition::brushDefinition(const string& name, const Vec4f& color, const map<string, SpawnFlag>& flags, const vector<Property*> properties, const string& description) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = TB_EDT_BRUSH;
            definition->name = name;
            definition->color = color;
            definition->flags = flags;
            definition->properties = properties;
            definition->description = description;
            return definition;
        }

        EntityDefinition::~EntityDefinition() {
            while(!properties.empty()) delete properties.back(), properties.pop_back();
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

        ModelProperty* EntityDefinition::modelPropertyForEntity(const Entity& entity) const {
            ModelProperty* defaultProperty = NULL;
            ModelProperty* specificProperty = NULL;
            for (int i = 0; i < properties.size() && specificProperty == NULL; i++) {
                Property* property = properties[i];
                if (property->type == TB_EDP_MODEL) {
                    ModelProperty* modelProperty = static_cast<ModelProperty*>(property);
                    if (modelProperty->flagName.empty())
                        defaultProperty = modelProperty;
                    else if (flagSetOnEntity(modelProperty->flagName, entity))
                        specificProperty = modelProperty;
                }
            }

            return specificProperty != NULL ? specificProperty : defaultProperty;
        }

        ModelProperty* EntityDefinition::defaultModelProperty() const {
            for (int i = 0; i < properties.size(); i++) {
                Property* property = properties[i];
                if (property->type == TB_EDP_MODEL) {
                    ModelProperty* modelProperty = static_cast<ModelProperty*>(property);
                    if (modelProperty->flagName.empty())
                        return modelProperty;
                }
            }

            return NULL;
        }

#pragma mark EntityDefinitionManager

        EntityDefinitionMap* EntityDefinitionManager::sharedManagers = NULL;

        EntityDefinitionManager::EntityDefinitionManager(const string& path) {
            clock_t start = clock();
            IO::EntityDefinitionParser parser(path);
            EntityDefinition* definition = NULL;
            while ((definition = parser.nextDefinition()) != NULL) {
                m_definitions[definition->name] = definition;
                m_definitionsByName.push_back(definition);
            }

            sort(m_definitionsByName.begin(), m_definitionsByName.end(), compareByName);
            fprintf(stdout, "Loaded %s in %f seconds\n", path.c_str(), (clock() - start) / CLK_TCK / 10000.0f);
        }

        EntityDefinitionManager::~EntityDefinitionManager() {
            while(!m_definitionsByName.empty()) delete m_definitionsByName.back(), m_definitionsByName.pop_back();
        }

        EntityDefinitionManager* EntityDefinitionManager::sharedManager(const string& path) {
            map<string, EntityDefinitionManager*>::iterator it = sharedManagers->managers.find(path);
            if (it != sharedManagers->managers.end())
                return it->second;

            EntityDefinitionManager* instance = new EntityDefinitionManager(path);
            sharedManagers->managers[path] = instance;
            return instance;
        }

        EntityDefinition* EntityDefinitionManager::definition(const string& name) const {
            map<const string, EntityDefinition*>::const_iterator it = m_definitions.find(name);
            if (it == m_definitions.end())
                return NULL;
            return it->second;
        }

        const vector<EntityDefinition*> EntityDefinitionManager::definitions() const {
            return m_definitionsByName;
        }

        const vector<EntityDefinition*> EntityDefinitionManager::definitions(EEntityDefinitionType type) const {
            return definitions(type, ES_NAME);
        }

        const vector<EntityDefinition*>EntityDefinitionManager::definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const {
            vector<EntityDefinition*> definitionsOfType;
            for (int i = 0; i < m_definitionsByName.size(); i++)
                if (m_definitionsByName[i]->type == type)
                    definitionsOfType.push_back(m_definitionsByName[i]);
            if (criterion == ES_USAGE)
                sort(definitionsOfType.begin(), definitionsOfType.end(), compareByUsage);
            return definitionsOfType;
        }
    }
}
