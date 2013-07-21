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

#ifndef __TrenchBroom__Map__
#define __TrenchBroom__Map__

#include "SharedPointer.h"
#include "Model/Entity.h"
#include "Model/ModelTypes.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        class Map {
        private:
            EntityList m_entities;
            mutable Entity* m_worldspawn;
        public:
            Map();
            ~Map();
            
            const EntityList& entities() const;
            void addEntity(Entity* entity);
            Entity* worldspawn() const;

            template <class Operator, class Filter>
            inline void eachObject(const Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    if (filter(entity))
                        op(entity);
                    entity->eachBrush(op, filter);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachObject(Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    if (filter(entity))
                        op(entity);
                    entity->eachBrush(op, filter);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachEntity(const Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    if (filter(entity))
                        op(entity);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachEntity(Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    if (filter(entity))
                        op(entity);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachBrush(const Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    entity->eachBrush(op, filter);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachBrush(Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    entity->eachBrush(op, filter);
                }
            }
            
            template <class Operator, class Filter>
            inline void eachBrushFace(const Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    entity->eachBrushFace(op, filter);
                }
            }

            template <class Operator, class Filter>
            inline void eachBrushFace(Operator& op, const Filter& filter) {
                EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity* entity = *it;
                    entity->eachBrushFace(op, filter);
                }
            }
        private:
            Entity* findWorldspawn() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
