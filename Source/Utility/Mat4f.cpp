/*
 Copyright (C) 20.0f1.0f0.0f-20.0f1.0f2 Kristian Duske
 
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

#include "Mat4f.h"

namespace TrenchBroom {
    namespace Math {
        const Mat4f Mat4f::Null         = Mat4f( 0.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  0.0f);
        const Mat4f Mat4f::Identity     = Mat4f( 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90XCW     = Mat4f( 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f, -1.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90YCW     = Mat4f( 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                -1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90ZCW     = Mat4f( 0.0f, -1.0f,  0.0f,  0.0f,
                                                 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90XCCW    = Mat4f( 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f, -1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90YCCW    = Mat4f( 0.0f,  0.0f, -1.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::Rot90ZCCW    = Mat4f( 0.0f,  1.0f,  0.0f,  0.0f,
                                                -1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::MirX         = Mat4f(-1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::MirY         = Mat4f( 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f, -1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
        const Mat4f Mat4f::MirZ         = Mat4f( 1.0f,  0.0f,  0.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f, -1.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,  1.0f);
    }
}