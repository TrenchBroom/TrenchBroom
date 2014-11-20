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

#ifndef __TrenchBroom__PickRay__
#define __TrenchBroom__PickRay__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace View {
        class PickRay : public Ray3 {
        public:
            Vec3 viewDir;
        public:
            PickRay();
            PickRay(const Ray3& ray, const Vec3& viewDir);
            PickRay(const Vec3& origin, const Vec3& direction, const Vec3& viewDir);

            FloatType perpendicularIntersectWithSphere(const Vec3& center, FloatType radius) const;
            Ray3 perpendicularRay(const Vec3& point) const;
            Vec3 perpendicularOrigin(const Vec3& point) const;
            FloatType perpendicularDistance(const Vec3& point) const;
        };
    }
}

#endif /* defined(__TrenchBroom__PickRay__) */
