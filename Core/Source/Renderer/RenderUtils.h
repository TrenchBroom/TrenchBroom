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

#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        void glVertexV3f(const Vec3f& vertex);
        void glColorV4f(const Vec4f& color);
        void glColorV4f(const Vec4f& color, float blendFactor);
        void glSetEdgeOffset(float f);
        void glResetEdgeOffset();
    }
}

#endif
