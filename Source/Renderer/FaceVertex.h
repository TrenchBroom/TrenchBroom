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

#ifndef __TrenchBroom__FaceVertex__
#define __TrenchBroom__FaceVertex__

#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
#if defined _WIN32
#pragma pack(push,1)
#endif
        struct FaceVertex {
            typedef std::vector<FaceVertex> List;
            
            float px, py, pz;
            float nx, ny, nz;
            float ts, tt;
            
            FaceVertex(const Vec3f& position, const Vec3f& normal, const Vec2f& texCoord) :
            px(position.x),
            py(position.y),
            pz(position.z),
            nx(normal.x),
            ny(normal.y),
            nz(normal.z),
            ts(texCoord.x),
            tt(texCoord.y) {}
            
            FaceVertex() {}
            
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif
    }
}

#endif /* defined(__TrenchBroom__FaceVertex__) */
