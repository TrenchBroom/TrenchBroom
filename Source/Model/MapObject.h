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

#ifndef __TrenchBroom__MapObject__
#define __TrenchBroom__MapObject__

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Filter;
        class PickResult;
        
        class MapObject {
        private:
            int m_uniqueId;
        public:
            enum Type {
                EntityObject,
                BrushObject
            };

            MapObject() {
                static int currentId = 1;
                m_uniqueId = currentId++;
            }
            
            virtual ~MapObject() {}
            
            inline int uniqueId() const {
                return m_uniqueId;
            }
            
            virtual const Vec3f& center() const = 0;
            virtual const BBox& bounds() const = 0;
            virtual Type objectType() const = 0;
            virtual void pick(const Ray& ray, PickResult& pickResults, Filter& filter) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapObject__) */
