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

#include "Grid.h"
#include "Utilities/VecMath.h"
#include "Utilities/Console.h"
#include <cmath>

namespace TrenchBroom {
    namespace Controller {
        unsigned int Grid::size() const {
            return m_size;
        }
        
        void Grid::setSize(unsigned int size) {
            if (m_size == size)
                return;
            
            if (size > MaxSize)
                m_size = MaxSize;
            else
                m_size = size;
            gridDidChange(*this);
        }

        unsigned int Grid::actualSize() const {
            if (m_snap)
                return 1 << m_size;
            return 1;
        }

        float Grid::snap(float f) {
            int actSize = actualSize();
            return actSize * Math::fround(f / actSize);
        }

        float Grid::snapDown(float f) {
            int actSize = actualSize();
            return actSize * floor(f / actSize);
        }
        
        float Grid::snapUp(float f) {
            int actSize = actualSize();
            return actSize * ceil(f / actSize);
        }

        Vec3f Grid::snap(const Vec3f& p) {
            return Vec3f(snap(p.x), snap(p.y), snap(p.z));
        }
        
        Vec3f Grid::snapUp(const Vec3f& p) {
            return Vec3f(snapUp(p.x), snapUp(p.y), snapUp(p.z));
        }
        
        Vec3f Grid::snapDown(const Vec3f& p) {
            return Vec3f(snapDown(p.x), snapDown(p.y), snapDown(p.z));
        }
        
        Vec3f Grid::snapTowards(const Vec3f& p, const Vec3f& d) {
            Vec3f result;
            if (Math::fpos(d.x))        result.x = snapUp(p.x);
            else if(Math::fneg(d.x))    result.x = snapDown(p.x);
            else                        result.x = snap(p.x);
            if (Math::fpos(d.y))        result.y = snapUp(p.y);
            else if(Math::fneg(d.y))    result.y = snapDown(p.y);
            else                        result.y = snap(p.y);
            if (Math::fpos(d.z))        result.z = snapUp(p.z);
            else if(Math::fneg(d.z))    result.z = snapDown(p.z);
            else                        result.z = snap(p.z);
            return result;
        }

        Vec3f Grid::moveDelta(const BBox& bounds, const BBox& worldBounds, const Vec3f& referencePoint, const Vec3f& curMousePoint) {
            Vec3f delta = curMousePoint - referencePoint;
            for (int i = 0; i < 3; i++) {
                float low  = snap(bounds.min[i] + delta[i]) - bounds.min[i];
                float high = snap(bounds.max[i] + delta[i]) - bounds.max[i];
                
                if (low != 0 && high != 0)
                    delta[i] = fabsf(high) < fabsf(low) ? high : low;
                else if (low != 0)
                    delta[i] = low;
                else if (high != 0)
                    delta[i] = high;
                else
                    delta[i] = 0;
                
            }
            
            if ((curMousePoint - referencePoint).lengthSquared() < (curMousePoint - (referencePoint + delta)).lengthSquared())
                delta = Null3f;
            
            return delta;
        }
        
    }
}

