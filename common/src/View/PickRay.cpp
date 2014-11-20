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

#include "PickRay.h"

namespace TrenchBroom {
    namespace View {
        PickRay::PickRay() {}

        PickRay::PickRay(const Ray3& ray, const Vec3& i_viewDir) :
        Ray3(ray),
        viewDir(i_viewDir) {}

        PickRay::PickRay(const Vec3& i_origin, const Vec3& i_direction, const Vec3& i_viewDir) :
        Ray3(i_origin, i_direction),
        viewDir(i_viewDir) {}

        FloatType PickRay::perpendicularIntersectWithSphere(const Vec3& center, const FloatType radius) const {
            const Ray3 perpRay = perpendicularRay(center);
            const FloatType dist = perpRay.intersectWithSphere(center, radius);
            if (Math::isnan(dist))
                return dist;
            const Vec3 hitVec = perpRay.pointAtDistance(dist) - origin;
            return hitVec.dot(direction);
        }

        Ray3 PickRay::perpendicularRay(const Vec3& point) const {
            return Ray3(perpendicularOrigin(point), viewDir);
        }
        
        Vec3 PickRay::perpendicularOrigin(const Vec3& point) const {
            return point - perpendicularDistance(point) * viewDir;
        }
        
        FloatType PickRay::perpendicularDistance(const Vec3& point) const {
            return (point - origin).dot(viewDir);
        }
    }
}
