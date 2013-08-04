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

#ifndef TrenchBroom_ModelUtils_h
#define TrenchBroom_ModelUtils_h

#include "Model/ModelTypes.h"
#include "Model/Map.h"
#include "Model/Entity.h"
#include "Model/Brush.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        struct MatchAllFilter {
            inline bool operator()(Object* entity) const {
                return true;
            }
            
            inline bool operator()(Entity* entity) const {
                return true;
            }
            
            inline bool operator()(Brush* brush) const {
                return true;
            }

            inline bool operator()(Brush* brush, BrushFace* face) const {
                return true;
            }
        };
        
        template <typename Iter, class Operator, class Filter>
        inline void eachObject(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                Object* object = *cur;
                if (filter(object))
                    op(object);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        inline void eachObject(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                Object* object = *cur;
                if (filter(object))
                    op(object);
                ++cur;
            }
        }
        
        template <typename Iter, class Operator, class Filter>
        inline void eachEntity(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                Entity* entity = *cur;
                if (filter(entity))
                    op(entity);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        inline void eachEntity(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                Entity* entity = *cur;
                if (filter(entity))
                    op(entity);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        inline void eachBrush(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                Brush* brush = *cur;
                if (filter(brush))
                    op(brush);
                ++cur;
            }
        }
        
        template <typename Iter, class Operator, class Filter>
        inline void eachBrush(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                Brush* brush = *cur;
                if (filter(brush))
                    op(brush);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        void eachFace(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                BrushFace* face = *cur;
                Brush* brush = face->parent();
                if (filter(brush, face))
                    op(brush, face);
                ++cur;
            }
        }
        
        template <typename Iter, class Operator, class Filter>
        void eachFace(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                BrushFace* face = *cur;
                Brush* brush = face->parent();
                if (filter(brush, face))
                    op(brush, face);
                ++cur;
            }
        }

        /* Operators for map */
        template <class Operator, class Filter>
        inline void eachObject(Map& map, const Operator& op, const Filter& filter) {
            eachEntity(map, op, filter);
            eachBrush(map, op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachObject(Map& map, Operator& op, const Filter& filter) {
            eachEntity(map, op, filter);
            eachBrush(map, op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachEntity(Map& map, const Operator& op, const Filter& filter) {
            eachEntity(map.entities(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachEntity(Map& map, Operator& op, const Filter& filter) {
            eachEntity(map.entities(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(Map& map, const Operator& op, const Filter& filter) {
            eachBrush(map.entities(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(Map& map, Operator& op, const Filter& filter) {
            eachBrush(map.entities(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachFace(Map& map, const Operator& op, const Filter& filter) {
            eachFace(map.entities(), op, filter);
        }

        template <class Operator, class Filter>
        inline void eachFace(Map& map, Operator& op, const Filter& filter) {
            eachFace(map.entities(), op, filter);
        }
        
        /* Operators for entity */
        template <class Operator, class Filter>
        inline void eachBrush(Entity& entity, const Operator& op, const Filter& filter) {
            eachBrush(entity.brushes(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(Entity& entity, Operator& op, const Filter& filter) {
            eachBrush(entity.brushes(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachFace(Entity& entity, const Operator& op, const Filter& filter) {
            eachFace(entity.brushes(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachFace(Entity& entity, Operator& op, const Filter& filter) {
            eachFace(entity.brushes(), op, filter);
        }
        
        /* Operations for brush */
        template <class Operator, class Filter>
        inline void eachFace(Brush& brush, const Operator& op, const Filter& filter) {
            eachFace(brush.faces(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachFace(Brush& brush, Operator& op, const Filter& filter) {
            eachFace(brush.faces(), op, filter);
        }
        
        /* Operations for object collections */
        template <class Operator, class Filter>
        inline void eachObject(const ObjectList& objects, const Operator& op, const Filter& filter) {
            eachObject(objects.begin(), objects.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachObject(const ObjectList& objects, Operator& op, const Filter& filter) {
            eachObject(objects.begin(), objects.end(), op, filter);
        }

        /* Operations for entity collections */
        template <class Operator, class Filter>
        inline void eachEntity(const EntityList& entities, const Operator& op, const Filter& filter) {
            eachEntity(entities.begin(), entities.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachEntity(const EntitySet& entities, const Operator& op, const Filter& filter) {
            eachEntity(entities.begin(), entities.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachEntity(const EntityList& entities, Operator& op, const Filter& filter) {
            eachEntity(entities.begin(), entities.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachEntity(const EntitySet& entities, Operator& op, const Filter& filter) {
            eachEntity(entities.begin(), entities.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const EntityList& entities, const Operator& op, const Filter& filter) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachBrush(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const EntitySet& entities, const Operator& op, const Filter& filter) {
            Model::EntitySet::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachBrush(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const EntityList& entities, Operator& op, const Filter& filter) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachBrush(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const EntitySet& entities, Operator& op, const Filter& filter) {
            Model::EntitySet::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachBrush(entity->brushes(), op, filter);
            }
        }

        template <class Operator, class Filter>
        void eachFace(const Model::EntityList& entities, const Operator& op, const Filter& filter) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachFace(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::EntitySet& entities, const Operator& op, const Filter& filter) {
            Model::EntitySet::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachFace(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::EntityList& entities, Operator& op, const Filter& filter) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachFace(entity->brushes(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::EntitySet& entities, Operator& op, const Filter& filter) {
            Model::EntitySet::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                eachFace(entity->brushes(), op, filter);
            }
        }

        /* Operations for brush collections */
        template <class Operator, class Filter>
        inline void eachBrush(const BrushList& brushes, const Operator& op, const Filter& filter) {
            eachBrush(brushes.begin(), brushes.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const BrushSet& brushes, const Operator& op, const Filter& filter) {
            eachBrush(brushes.begin(), brushes.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const BrushList& brushes, Operator& op, const Filter& filter) {
            eachBrush(brushes.begin(), brushes.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        inline void eachBrush(const BrushSet& brushes, Operator& op, const Filter& filter) {
            eachBrush(brushes.begin(), brushes.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::BrushList& brushes, const Operator& op, const Filter& filter) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                eachFace(brush->faces(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::BrushSet& brushes, const Operator& op, const Filter& filter) {
            Model::BrushSet::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                eachFace(brush->faces(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::BrushList& brushes, Operator& op, const Filter& filter) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                eachFace(brush->faces(), op, filter);
            }
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::BrushSet& brushes, Operator& op, const Filter& filter) {
            Model::BrushSet::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                eachFace(brush->faces(), op, filter);
            }
        }

        /* Operations for face collections */
        template <class Operator, class Filter>
        void eachFace(const Model::BrushFaceList& faces, const Operator& op, const Filter& filter) {
            eachFace(faces.begin(), faces.end(), op, filter);
        }
        
        template <class Operator, class Filter>
        void eachFace(const Model::BrushFaceList& faces, Operator& op, const Filter& filter) {
            eachFace(faces.begin(), faces.end(), op, filter);
        }
    }
}
#endif
