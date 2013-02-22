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

#include "IO/FileManager.h"
#include "IO/StandardDefinitionParser.h"
#include "IO/mmapped_fstream.h"
#include "Utility/Preferences.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        EntityDefinitionManager::EntityDefinitionManager() {}
        
        EntityDefinitionManager::~EntityDefinitionManager() {
            clear();
        }
        
        StringList EntityDefinitionManager::builtinDefinitionFiles() {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            return fileManager.directoryContents(resourcePath, "def");
        }

        void EntityDefinitionManager::load(const String& path) {
            mmapped_fstream stream(path.c_str(), std::ios::in);
            IO::StandardDefinitionParser parser(stream);
            
            EntityDefinition* definition = NULL;
            while ((definition = parser.nextDefinition()) != NULL) {
                m_entityDefinitions[definition->name()] = definition;
            }
            
            m_path = path;
        }
        
        void EntityDefinitionManager::clear() {
            EntityDefinitionMap::iterator it, end;
            for (it = m_entityDefinitions.begin(), end = m_entityDefinitions.end(); it != end; ++it)
                delete it->second;
            m_entityDefinitions.clear();
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

        EntityDefinitionManager::EntityDefinitionGroups EntityDefinitionManager::groups(EntityDefinition::Type type, SortOrder order) {
            EntityDefinitionGroups groups;
            EntityDefinitionList list = definitions(type, order);
            
            EntityDefinitionList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                EntityDefinition* definition = *it;
                const String groupName = definition->groupName();
                groups[groupName].push_back(definition);
            }
            
            return groups;
        }
    }
}
