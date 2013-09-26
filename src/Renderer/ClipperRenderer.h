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

#include "Color.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Circle.h"
#include "Renderer/Sphere.h"
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
            const View::Clipper& m_clipper;
            BrushRenderer m_frontRenderer;
            BrushRenderer m_backRenderer;
            Vbo m_vbo;
        public:
            ClipperRenderer(const View::Clipper& clipper);
            
            void renderClipPoints(RenderContext& renderContext);
            void renderHighlight(RenderContext& renderContext, const size_t index);
            void renderBrushes(RenderContext& renderContext);
            void renderCurrentPoint(RenderContext& renderContext, const Vec3& position);
            
            void setBrushes(const Model::BrushList& frontBrushes, const Model::BrushList& backBrushes);
        private:
            void renderPointHandles(RenderContext& renderContext, const Vec3::List& positions, Sphere& pointHandle);
            void renderPointHandle(const Vec3& position, ActiveShader& shader, Sphere& pointHandle, const Color& color, const Color& occludedColor);
            void renderPlaneIndicators(RenderContext& renderContext, VertexArray& lineArray, VertexArray& triangleArray);
            Sphere makePointHandle();
            Circle makePointHandleHighlight();
            VertexArray makeLineArray(const Vec3::List& positions);
            VertexArray makeTriangleArray(const Vec3::List& positions);
            void setupBrushRenderer(BrushRenderer& renderer, const bool keep);
        };
    }
}

#endif /* defined(__TrenchBroom__ClipperRenderer__) */
