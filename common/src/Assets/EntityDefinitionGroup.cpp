/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "EntityDefinitionGroup.h"

namespace TrenchBroom {
    namespace Assets {
        EntityDefinitionGroup::EntityDefinitionGroup(const String& name, const EntityDefinitionArray& definitions) :
        m_name(name),
        m_definitions(definitions) {}
        
        const String& EntityDefinitionGroup::name() const {
            return m_name;
        }

        const String EntityDefinitionGroup::displayName() const {
            if (m_name.empty())
                return "Misc";
            return StringUtils::capitalize(m_name);
        }

        const EntityDefinitionArray& EntityDefinitionGroup::definitions() const {
            return m_definitions;
        }

        EntityDefinitionArray EntityDefinitionGroup::definitions(const EntityDefinition::Type type, const EntityDefinition::SortOrder order) const {
            return EntityDefinition::filterAndSort(m_definitions, type, order);
        }
    }
}
