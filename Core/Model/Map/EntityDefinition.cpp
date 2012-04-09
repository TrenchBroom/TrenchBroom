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
#include "EntityDefinitionParser.h"

namespace TrenchBroom {
    namespace Model {
        bool compareByName(const EntityDefinition* def1, const EntityDefinition* def2) {
            return def1->name <= def2->name;
        }
        
        bool compareByUsage(const EntityDefinition* def1, const EntityDefinition* def2) {
            return def1->usageCount <= def2->usageCount;
        }
        
        EntityDefinition* EntityDefinition::baseDefinition(const string& name, const map<string, SpawnFlag>& flags, const vector<Property*>& properties) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = EDT_BASE;
            definition->name = name;
            definition->flags = flags;
            definition->properties = properties;
            return definition;
        }
        
        EntityDefinition* EntityDefinition::pointDefinition(const string& name, const Vec4f& color, const BBox& bounds, const map<string, SpawnFlag>& flags, const vector<Property*>& properties, const string& description) {
            EntityDefinition* definition = new EntityDefinition();
            definition->type = EDT_POINT;
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
            definition->type = EDT_BRUSH;
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
            static map<string, EntityDefinitionManager*> instances;
            map<string, EntityDefinitionManager*>::iterator it = instances.find(path);
            if (it != instances.end())
                return it->second;
            
            EntityDefinitionManager* instance = new EntityDefinitionManager(path);
            instances[path] = instance;
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