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

#include <iterator>

namespace TrenchBroom {
    namespace Model {
        inline Brush* createBrushFromBounds(const Map& map, const BBox3& worldBounds, const BBox3& brushBounds, const String& textureName) {
            const Vec3 size = brushBounds.size();
            const Vec3 x = Vec3(size.x(), 0.0, 0.0);
            const Vec3 y = Vec3(0.0, size.y(), 0.0);
            const Vec3 z = Vec3(0.0, 0.0, size.z());
            
            // east, west, front, back, top, bottom
            BrushFaceList faces(6);
            faces[0] = map.createFace(brushBounds.min, brushBounds.min + y, brushBounds.min + z, textureName);
            faces[1] = map.createFace(brushBounds.max, brushBounds.max - z, brushBounds.max - y, textureName);
            faces[2] = map.createFace(brushBounds.min, brushBounds.min + z, brushBounds.min + x, textureName);
            faces[3] = map.createFace(brushBounds.max, brushBounds.max - x, brushBounds.max - z, textureName);
            faces[4] = map.createFace(brushBounds.max, brushBounds.max - y, brushBounds.max - x, textureName);
            faces[5] = map.createFace(brushBounds.min, brushBounds.min + x, brushBounds.min + y, textureName);
            
            return map.createBrush(worldBounds, faces);
        }
        
        struct MatchAll {
            inline bool operator()(const Object* entity) const {
                return true;
            }
            
            inline bool operator()(const Entity* entity) const {
                return true;
            }
            
            inline bool operator()(const Brush* brush) const {
                return true;
            }
            
            inline bool operator()(const BrushFace* face) const {
                return true;
            }
        };
        
        template <class Container>
        struct ExtractObjectsByType {
        private:
            Object::Type m_type;
        public:
            Container result;

            ExtractObjectsByType(const Object::Type type) :
            m_type(type) {}
            
            inline bool operator()(Object* object) {
                if (object->type() == m_type)
                    result.insert(result.end(), static_cast<Entity*>(object));
                return true;
            }
        };
        
        template <typename Iter, class Operator, class Filter>
        inline void each(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    op(*cur);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        inline void each(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    op(*cur);
                ++cur;
            }
        }
        
        template <typename Iter, class Filter>
        inline bool any(Iter cur, Iter end, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    return true;
                ++cur;
            }
            return false;
        }
        
        template <typename InputIter, class Filter, typename OutputIter>
        inline void filter(InputIter cur, InputIter end, const Filter& filter, OutputIter output) {
            while (cur != end) {
                if (filter(*cur))
                    *output++ = *cur;
                ++cur;
            }
        }
        
        template <template <typename> class Iter, class Container>
        inline Container filterObjectsByType(Iter<Object*> cur, Iter<Object*> end, const Object::Type type) {
            ExtractObjectsByType<Container> extract(type);
            each(cur, end, extract, MatchAll());
            return extract.result;
        }
    }
}
#endif
