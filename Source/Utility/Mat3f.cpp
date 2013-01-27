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

#include "Mat3f.h"

namespace TrenchBroom {
    namespace Math {
        const Mat3f Mat3f::Null         = Mat3f( 0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f,
                                                 0.0f,  0.0f,  0.0f);
        const Mat3f Mat3f::Identity     = Mat3f( 1.0f,  0.0f,  0.0f,
                                                 0.0f,  1.0f,  0.0f,
                                                 0.0f,  0.0f,  1.0f);
        const Mat3f Mat3f::YIQToRGB     = Mat3f( 1.0f,  0.9563f,  0.6210f,
                                                 1.0f, -0.2721f, -0.6474f,
                                                 1.0f, -1.1070f,  1.7046f);
        const Mat3f Mat3f::RGBToYIQ     = Mat3f( 0.299f,     0.587f,     0.114f,
                                                 0.595716f, -0.274453f, -0.321263f,
                                                 0.211456f, -0.522591f,  0.311135f);
    }
}
