/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Ensure.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "IO/EntityDefinitionLoader.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        EntityDefinitionManager::~EntityDefinitionManager() {
            clear();
        }

        void EntityDefinitionManager::loadDefinitions(const IO::Path& path, const IO::EntityDefinitionLoader& loader, IO::ParserStatus& status) {
            setDefinitions(loader.loadEntityDefinitions(status, path));
        }

        void EntityDefinitionManager::setDefinitions(const std::vector<EntityDefinition*>& newDefinitions) {
            clear();

            m_definitions = newDefinitions;

            updateIndices();
            updateGroups();
            updateCache();
            bindObservers();
        }

        void EntityDefinitionManager::clear() {
            clearCache();
            clearGroups();
            kdl::vec_clear_and_delete(m_definitions);
        }

        EntityDefinition* EntityDefinitionManager::definition(const Model::AttributableNode* attributable) const {
            ensure(attributable != nullptr, "attributable is null");
            return definition(attributable->entity().classname());
        }

        EntityDefinition* EntityDefinitionManager::definition(const std::string& classname) const {
            auto it = m_cache.find(classname);
            if (it == std::end(m_cache)) {
                return nullptr;
            } else {
                return it->second;
            }
        }

        std::vector<EntityDefinition*> EntityDefinitionManager::definitions(const EntityDefinitionType type, const EntityDefinitionSortOrder order) const {
            return EntityDefinition::filterAndSort(m_definitions, type, order);
        }

        const std::vector<EntityDefinition*>& EntityDefinitionManager::definitions() const {
            return m_definitions;
        }

        const std::vector<EntityDefinitionGroup>& EntityDefinitionManager::groups() const {
            return m_groups;
        }

        void EntityDefinitionManager::updateIndices() {
            for (size_t i = 0; i < m_definitions.size(); ++i) {
                m_definitions[i]->setIndex(i + 1);
            }
        }

        void EntityDefinitionManager::updateGroups() {
            clearGroups();

            std::map<std::string, std::vector<EntityDefinition*>> groupMap;

            for (auto* definition : m_definitions) {
                const std::string groupName = definition->groupName();
                groupMap[groupName].push_back(definition);
            }

            for (auto& [groupName, definitions] : groupMap) {
                m_groups.push_back(EntityDefinitionGroup(groupName, std::move(definitions)));
            }
        }

        void EntityDefinitionManager::updateCache() {
            clearCache();
            for (EntityDefinition* definition : m_definitions) {
                m_cache[definition->name()] = definition;
            }
        }

        void EntityDefinitionManager::bindObservers() {
            for (EntityDefinition* definition : m_definitions) {
                definition->usageCountDidChangeNotifier.addObserver(usageCountDidChangeNotifier);
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
