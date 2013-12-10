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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityPropertyIndex__
#define __TrenchBroom__EntityPropertyIndex__

#include "StringUtils.h"
#include "Model/EntityProperties.h"
#include "Model/ModelTypes.h"
#include "StringIndex.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class EntityPropertyQuery {
        public:
            typedef enum {
                Exact,
                Prefix,
                Numbered,
                Any
            } Type;
        private:
            Type m_type;
            String m_pattern;
        public:
            static EntityPropertyQuery exact(const String& pattern);
            static EntityPropertyQuery prefix(const String& pattern);
            static EntityPropertyQuery numbered(const String& pattern);
            static EntityPropertyQuery any();

            EntitySet execute(const StringIndex<Entity*>& index) const;
        private:
            EntityPropertyQuery(Type type, const String& pattern = "");
        };
        
        class EntityPropertyIndex {
        private:
            StringIndex<Entity*> m_keyIndex;
            StringIndex<Entity*> m_valueIndex;
        public:
            void addEntity(Entity* entity);
            void removeEntity(Entity* entity);
            
            void addEntityProperty(Entity* entity, const EntityProperty& property);
            void removeEntityProperty(Entity* entity, const EntityProperty& property);
            
            EntityList findEntities(const EntityPropertyQuery& keyQuery, const EntityPropertyQuery& valueQuery) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyIndex__) */
