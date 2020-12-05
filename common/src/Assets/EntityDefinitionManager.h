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

#pragma once

#include "Notifier.h"

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class EntityDefinitionLoader;
        class ParserStatus;
        class Path;
    }

    namespace Model {
        class AttributableNode;
    }

    namespace Assets {
        class EntityDefinition;
        class EntityDefinitionGroup;
        enum class EntityDefinitionSortOrder;
        enum class EntityDefinitionType;

        class EntityDefinitionManager {
        private:
            using Cache = std::map<std::string, EntityDefinition*>;
            std::vector<EntityDefinition*> m_definitions;
            std::vector<EntityDefinitionGroup> m_groups;
            Cache m_cache;
        public:
            Notifier<> usageCountDidChangeNotifier;
        public:
            ~EntityDefinitionManager();

            void loadDefinitions(const IO::Path& path, const IO::EntityDefinitionLoader& loader, IO::ParserStatus& status);
            void setDefinitions(const std::vector<EntityDefinition*>& newDefinitions);
            void clear();

            EntityDefinition* definition(const Model::AttributableNode* attributable) const;
            EntityDefinition* definition(const std::string& classname) const;
            std::vector<EntityDefinition*> definitions(EntityDefinitionType type, EntityDefinitionSortOrder order) const;
            const std::vector<EntityDefinition*>& definitions() const;

            const std::vector<EntityDefinitionGroup>& groups() const;
        private:
            void updateIndices();
            void updateGroups();
            void updateCache();
            void bindObservers();
            void clearCache();
            void clearGroups();
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionManager) */
