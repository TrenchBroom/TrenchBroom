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

#ifndef TrenchBroom_Ray_h
#define TrenchBroom_Ray_h

#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class Ray {
        public:
            Vec3f origin;
            Vec3f direction;

            Ray() : origin(Vec3f::Null), direction(Vec3f::Null) {}

            Ray(const Vec3f& origin, const Vec3f& direction) : origin(origin), direction(direction) {}

            inline const Vec3f pointAtDistance(float distance) const {
                return Vec3f(origin.x + direction.x * distance,
                             origin.y + direction.y * distance,
                             origin.z + direction.z * distance);
            }

            inline PointStatus::Type pointStatus(const Vec3f& point) const {
                float dot = direction.dot(point - origin);
                if (dot >  Math::PointStatusEpsilon)
                    return PointStatus::PSAbove;
                if (dot < -Math::PointStatusEpsilon)
                    return PointStatus::PSBelow;
                return PointStatus::PSInside;
            }
        };
    }
}

#endif
