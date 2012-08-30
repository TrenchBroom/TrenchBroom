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

#include "EntityDefinitionManager.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        EntityDefinitionManager::EntityDefinitionManager() {}
        
        EntityDefinitionManager::~EntityDefinitionManager() {
            EntityDefinitionMap::iterator it, end;
            for (it = m_entityDefinitions.begin(), end = m_entityDefinitions.end(); it != end; ++it)
                delete it->second;
            m_entityDefinitions.clear();
        }
        
        bool EntityDefinitionManager::load(const String& path) {
        }

        EntityDefinition* EntityDefinitionManager::definition(const String& name) {
            EntityDefinitionMap::iterator it = m_entityDefinitions.find(name);
            return it != m_entityDefinitions.end() ? it->second : NULL;
        }
        
        EntityDefinitionList EntityDefinitionManager::definitions(EntityDefinition::Type type, SortOrder order) {
            EntityDefinitionList result;
            EntityDefinitionMap::iterator it, end;
            for (it = m_entityDefinitions.begin(), end = m_entityDefinitions.end(); it != end; ++it)
                if (it->second->type() == type)
                    result.push_back(it->second);
            if (order == Usage)
                std::sort(result.begin(), result.end(), CompareEntityDefinitionsByUsage());
            else
                std::sort(result.begin(), result.end(), CompareEntityDefinitionsByName());
            return result;
        }
    }
}
