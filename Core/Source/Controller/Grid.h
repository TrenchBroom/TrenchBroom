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

#ifndef TrenchBroom_Grid_h
#define TrenchBroom_Grid_h

#include "Utilities/Event.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class Grid {
        private:
            static const unsigned int MaxSize = 9;
            unsigned int m_size;
            bool m_snap;
        public:
            typedef Event<Grid&> GridEvent;

            Grid(unsigned int size) : m_size(size), m_snap(true) {}
            unsigned int size() const;
            void setSize(unsigned int size);
            unsigned int actualSize() const;
            
            float snap(float f);
            float snapDown(float f);
            float snapUp(float f);
            
            Vec3f moveDelta(const BBox& bounds, const BBox& worldBounds, const Vec3f& referencePoint, const Vec3f& curMousePoint);
            
            GridEvent gridDidChange;
        };
    }
}

#endif
