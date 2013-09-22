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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ClipperRenderer__
#define __TrenchBroom__ClipperRenderer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace View {
        class Clipper;
    }
    
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class VertexArray;
        
        class ClipperRenderer {
        private:
            Vbo m_vbo;
            const View::Clipper& m_clipper;
            bool m_hasCurrentPoint;
            Vec3 m_currentPoint;
        public:
            ClipperRenderer(const View::Clipper& clipper);
            
            void setCurrentPoint(const bool hasPoint, const Vec3& point = Vec3::Null);
            void render(RenderContext& renderContext);
        private:
            void renderHandles(RenderContext& renderContext);
            void renderPointHandles(RenderContext& renderContext, const Vec3::List& positions, VertexArray& handleArray);
            void renderPointHandle(const Vec3& position, ActiveShader& shader, VertexArray& array);
            void renderPlaneIndicators(RenderContext& renderContext, VertexArray& lineArray, VertexArray& triangleArray);
            VertexArray makeHandleArray();
            VertexArray makeLineArray(const Vec3::List& positions);
            VertexArray makeTriangleArray(const Vec3::List& positions);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipperRenderer__) */
