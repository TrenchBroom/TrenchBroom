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

#ifndef TrenchBroom_EntityDefinitionGroup
#define TrenchBroom_EntityDefinitionGroup

#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinition.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionGroup {
        public:
            typedef std::vector<EntityDefinitionGroup> List;
        private:
            String m_name;
            EntityDefinitionList m_definitions;
        public:
            EntityDefinitionGroup(const String& name, const EntityDefinitionList& definitions);
            
            size_t index() const;
            const String& name() const;
            const String displayName() const;
            const EntityDefinitionList& definitions() const;
            EntityDefinitionList definitions(EntityDefinition::Type type, const EntityDefinition::SortOrder order = EntityDefinition::Name) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionGroup) */
