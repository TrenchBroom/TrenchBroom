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
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Assets/EntityDefinition.h"
#include "Model/Game.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
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
