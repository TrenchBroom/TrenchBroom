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

#include "EntityDefinitionManager.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Game.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        bool EntityDefinitionManager::CompareByName::operator() (const EntityDefinition* left, const EntityDefinition* right) const {
            if (m_shortName)
                return left->shortName() < right->shortName();
            return left->name() < right->name();
        }

        inline bool EntityDefinitionManager::CompareByUsage::operator() (const EntityDefinition* left, const EntityDefinition* right) const {
            if (left->usageCount() == right->usageCount())
                return left->name() < right->name();
            return left->usageCount() > right->usageCount();
        }

        EntityDefinitionManager::~EntityDefinitionManager() {
            clear();
        }

        void EntityDefinitionManager::loadDefinitions(Model::GamePtr game, const IO::Path& path) {
            EntityDefinitionList newDefinitions = game->loadEntityDefinitions(path);
            VectorUtils::clearAndDelete(m_definitions);
            m_definitions = newDefinitions;
            updateCache();
        }

        void EntityDefinitionManager::clear() {
            clearCache();
            VectorUtils::clearAndDelete(m_definitions);
        }

        EntityDefinition* EntityDefinitionManager::definition(const Model::Entity* entity) const {
            assert(entity != NULL);
            return definition(entity->property(Model::PropertyKeys::Classname));
        }
        
        EntityDefinition* EntityDefinitionManager::definition(const Model::PropertyValue& classname) const {
            Cache::const_iterator it = m_cache.find(classname);
            if (it == m_cache.end())
                return NULL;
            return it->second;
        }

        EntityDefinitionList EntityDefinitionManager::definitions(const EntityDefinition::Type type, const SortOrder order) const {
            EntityDefinitionList result;
            EntityDefinitionList::const_iterator it, end;
            for (it = m_definitions.begin(), end = m_definitions.end(); it != end; ++it) {
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
        
        EntityDefinitionManager::EntityDefinitionGroups EntityDefinitionManager::groups(const EntityDefinition::Type type, const SortOrder order) const {
            EntityDefinitionGroups groups;
            EntityDefinitionList list = definitions(type, order);
            EntityDefinitionList ungrouped;
            
            EntityDefinitionList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                EntityDefinition* definition = *it;
                const String groupName = definition->groupName();
                if (groupName.empty())
                    ungrouped.push_back(definition);
                else
                    groups[groupName].push_back(definition);
            }
            
            for (it = ungrouped.begin(), end = ungrouped.end(); it != end; ++it) {
                EntityDefinition* definition = *it;
                const String shortName = StringUtils::capitalize(definition->shortName());
                EntityDefinitionGroups::iterator groupIt = groups.find(shortName);
                if (groupIt == groups.end())
                    groups["Misc"].push_back(definition);
                else
                    groupIt->second.push_back(definition);
            }
            
            EntityDefinitionGroups::iterator groupIt, groupEnd;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                EntityDefinitionList& definitions = groupIt->second;
                if (order == Usage)
                    std::sort(definitions.begin(), definitions.end(), CompareByUsage());
                else
                    std::sort(definitions.begin(), definitions.end(), CompareByName(true));
            }
            
            return groups;            }

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
    }
}
