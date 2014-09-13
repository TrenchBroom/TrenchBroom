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

#ifndef __TrenchBroom__RemoveObjectsQuery__
#define __TrenchBroom__RemoveObjectsQuery__

#include "Model/ModelTypes.h"
#include "Model/Object.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class AddObjectsQuery;
        
        class RemoveObjectsQuery : public ObjectVisitor {
        private:
            ObjectList m_parents;
            ObjectList m_objects;
            EntityList m_entities;
            BrushList m_brushes;
            
            typedef std::map<Entity*, size_t> BrushCountMap;
            BrushCountMap m_brushCounts;
        public:
            RemoveObjectsQuery();
            RemoveObjectsQuery(const AddObjectsQuery& addQuery);
            
            const ObjectList& parents() const;
            const ObjectList& objects() const;
            size_t objectCount() const;

            const EntityList& entities() const;
            const BrushList& brushes() const;

            template <typename I>
            void removeEntities(I cur, const I end) {
                while (cur != end) {
                    removeEntity(*cur);
                    ++cur;
                }
            }
            
            template <typename I>
            void removeBrushes(I cur, const I end) {
                while (cur != end) {
                    removeBrush(*cur);
                    ++cur;
                }
            }
            
            void removeEntity(Entity* entity);
            void removeBrush(Brush* brush);
            
            void clear();
            void clearAndDelete();
        private:
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__RemoveObjectsQuery__) */
