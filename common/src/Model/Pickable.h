/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_Pickable_h
#define TrenchBroom_Pickable_h

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
        
        class Pickable {
        public:
            virtual ~Pickable() {}
            virtual const vm::bbox3& bounds() const = 0;
            virtual void pick(const ray3& ray, PickResult& pickResult) const = 0;
            virtual FloatType intersectWithRay(const ray3& ray) const = 0;
        };
    }
}

#endif
