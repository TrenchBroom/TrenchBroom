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

#ifndef TrenchBroom_MapObject_h
#define TrenchBroom_MapObject_h

#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            MT_ENTITY,
            MT_BRUSH
        } EMapObjectType;

        class HitList;
        
        class MapObject {
        protected:
            int m_uniqueId;
        public:
            MapObject();
            virtual ~MapObject() {};
            int uniqueId() const;
            virtual const BBox& bounds() = 0;
            virtual EMapObjectType objectType() const = 0;
            virtual void pick(const Ray& ray, HitList& hits) = 0;
        };
    }
}

#endif
