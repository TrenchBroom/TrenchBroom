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

#include "EntityDefinitionManager.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "IO/EntityDefinitionLoader.h"
#include "Model/AttributableNode.h"
#include "Model/EntityAttributes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        EntityDefinitionManager::~EntityDefinitionManager() {
            clear();
        }

        void EntityDefinitionManager::loadDefinitions(const IO::Path& path, const IO::EntityDefinitionLoader& loader, IO::ParserStatus& status) {
            using std::swap;
            
            EntityDefinitionList newDefinitions = loader.loadEntityDefinitions(status, path);
            std::swap(m_definitions, newDefinitions);
            VectorUtils::clearAndDelete(newDefinitions);
            
            updateIndices();
            updateGroups();
            updateCache();
        }

        void EntityDefinitionManager::clear() {
            clearCache();
            clearGroups();
            VectorUtils::clearAndDelete(m_definitions);
        }

        EntityDefinition* EntityDefinitionManager::definition(const Model::AttributableNode* attributable) const {
            assert(attributable != NULL);
            return definition(attributable->attribute(Model::AttributeNames::Classname));
        }
        
        EntityDefinition* EntityDefinitionManager::definition(const Model::AttributeValue& classname) const {
            Cache::const_iterator it = m_cache.find(classname);
            if (it == m_cache.end())
                return NULL;
            return it->second;
        }

        EntityDefinitionList EntityDefinitionManager::definitions(const EntityDefinition::Type type, const EntityDefinition::SortOrder order) const {
            return EntityDefinition::filterAndSort(m_definitions, type, order);
        }
        
        const EntityDefinitionGroup::List& EntityDefinitionManager::groups() const {
            return m_groups;
        }

        void EntityDefinitionManager::updateIndices() {
            for (size_t i = 0; i < m_definitions.size(); ++i)
                m_definitions[i]->setIndex(i+1);
        }

        void EntityDefinitionManager::updateGroups() {
            clearGroups();
            
            typedef std::map<String, EntityDefinitionList> GroupMap;
            GroupMap groupMap;
            
            for (size_t i = 0; i < m_definitions.size(); ++i) {
                EntityDefinition* definition = m_definitions[i];
                const String groupName = definition->groupName();
                groupMap[groupName].push_back(definition);
            }
            
            GroupMap::const_iterator it, end;
            for (it = groupMap.begin(), end = groupMap.end(); it != end; ++it) {
                const String& groupName = it->first;
                const EntityDefinitionList& definitions = it->second;
                m_groups.push_back(EntityDefinitionGroup(groupName, definitions));
            }
        }

        void EntityDefinitionManager::updateCache() {
            clearCache();
            EntityDefinitionList::iterator it, end;
            for (it = m_definitions.begin(), end = m_definitions.end(); it != end; ++it) {
                EntityDefinition* definition = *it;
                m_cache[definition->name()] = definition;
            }
        }
        
        void EntityDefinitionManager::clearCache() {
            m_cache.clear();
        }

        void EntityDefinitionManager::clearGroups() {
            m_groups.clear();
        }
    }
}
