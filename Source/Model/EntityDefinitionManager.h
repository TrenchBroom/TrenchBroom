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
#ifndef __TrenchBroom__EntityDefinitionManager__
#define __TrenchBroom__EntityDefinitionManager__

#include "Model/EntityDefinition.h"
#include "Utility/String.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        
        class EntityDefinitionManager {
        public:
            enum class SortOrder {
                Name,
                Usage
            };
        protected:
            class CompareEntityDefinitionsByName {
            public:
                inline bool operator() (const EntityDefinition* left, const EntityDefinition* right) const {
                    return left->name().compare(right->name());
                }
            };

            class CompareEntityDefinitionsByUsage {
            public:
                inline bool operator() (const EntityDefinition* left, const EntityDefinition* right) const {
                    if (left->usageCount() == right->usageCount())
                        return left->name().compare(right->name());
                    return left->usageCount() > right->usageCount();
                }
            };
            
            typedef std::map<String, EntityDefinition*> EntityDefinitionMap;

            String m_name;
            EntityDefinitionMap m_entityDefinitions;
        public:
            EntityDefinitionManager();
            ~EntityDefinitionManager();
            
            bool load(const String& path);
            
            EntityDefinition* definition(const String& name);
            PointEntityDefinition::List definitions(EntityDefinition::Type type, SortOrder order = SortOrder::Name);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinitionManager__) */
