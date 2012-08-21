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

#include "Vec3f.h"

#include <limits>

namespace TrenchBroom {
    namespace Math {
        const Vec3f Vec3f::PosX = Vec3f( 1.0f,  0.0f,  0.0f);
        const Vec3f Vec3f::PosY = Vec3f( 0.0f,  1.0f,  0.0f);
        const Vec3f Vec3f::PosZ = Vec3f( 0.0f,  0.0f,  1.0f);
        const Vec3f Vec3f::NegX = Vec3f(-1.0f,  0.0f,  0.0f);
        const Vec3f Vec3f::NegY = Vec3f( 0.0f, -1.0f,  0.0f);
        const Vec3f Vec3f::NegZ = Vec3f( 0.0f,  0.0f, -1.0f);
        const Vec3f Vec3f::Null = Vec3f( 0.0f,  0.0f,  0.0f);
        const Vec3f Vec3f::NaN  = Vec3f(std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::quiet_NaN());
    }
}