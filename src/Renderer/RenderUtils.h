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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_RenderUtils_h
#define TrenchBroom_RenderUtils_h

#include "GL/GL.h"

namespace TrenchBroom {
    namespace Renderer {
        static const float EdgeOffset = 0.0001f;

        inline void glSetEdgeOffset(const float f) {
            glDepthRange(0.0f, 1.0f - EdgeOffset * f);
        }
        
        inline void glResetEdgeOffset() {
            glDepthRange(EdgeOffset, 1.0f);
        }
    }
}

#endif
