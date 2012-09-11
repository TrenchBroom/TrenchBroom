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


#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include "Model/Texture.h"
#include "Renderer/RenderTypes.h"
#include "Utility/Color.h"
#include "Utility/GLee.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;
        
        inline void glVertexV3f(const Vec3f& vertex) {
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        
        inline void glColorV4f(const Color& color) {
            glColor4f(color.x, color.y, color.z, color.w);
        }
        
        inline void glColorV4f(const Color& color, float blendFactor) {
            glColor4f(color.x, color.y, color.z, color.w * blendFactor);
        }
        
        inline void glSetEdgeOffset(float f) {
            glDepthRange(0.0f, 1.0f - EdgeOffset * f);
        }
        
        inline void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0f);
        }
    }
}

#endif
