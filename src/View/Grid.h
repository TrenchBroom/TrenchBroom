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

#ifndef __TrenchBroom__Grid__
#define __TrenchBroom__Grid__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace View {
        class Grid {
        public:
            static const size_t MaxSize = 9;
        private:
            size_t m_size;
            bool m_snap;
            bool m_visible;
        public:
            Grid(const size_t size);
            
            size_t size() const;
            void setSize(const size_t size);
            void incSize();
            void decSize();
            size_t actualSize() const;
            FloatType angle() const;
        
            bool visible() const;
            void toggleVisible();
            
            bool snap() const;
            void toggleSnap();
            
            FloatType snap(const FloatType f) const;
            FloatType snapAngle(const FloatType a) const;
            FloatType snapUp(const FloatType f, bool skip = false) const;
            FloatType snapDown(const FloatType f, bool skip = false) const;
            FloatType offset(const FloatType f) const;
            Vec3 snap(const Vec3& p) const;
            Vec3 snapUp(const Vec3& p, const bool skip = false) const;
            Vec3 snapDown(const Vec3& p, const bool skip = false) const;
            Vec3 snapTowards(const Vec3& p, const Vec3& d, const bool skip = false) const;
            Vec3 offset(const Vec3& p) const;
            Vec3 snap(const Vec3& p, const Plane3& onPlane) const;
            
            FloatType intersectWithRay(const Ray3& ray, const size_t skip) const;
            
            Vec3 moveDeltaForPoint(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDeltaForBounds(const Model::BrushFace& face, const BBox3& bounds, const BBox3& worldBounds, const Ray3& ray, const Vec3& position) const;
            Vec3 moveDelta(const BBox3& bounds, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDelta(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDelta(const Vec3& delta) const;
            Vec3 moveDelta(const Model::BrushFace& face, const Vec3& delta) const;
            Vec3 combineDeltas(const Vec3& delta1, const Vec3& delta2) const;
            Vec3 referencePoint(const BBox3& bounds);
        };
    }
}

#endif /* defined(__TrenchBroom__Grid__) */
