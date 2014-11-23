/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__PointHandleRenderer__
#define __TrenchBroom__PointHandleRenderer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class Vbo;
        
        class PointHandleRenderer : public Renderable {
        private:
            Vec3f::List m_points;
            Vec3f::List m_selectedPoints;
            Vec3f::List m_highlights;
            Circle m_handle;
            Circle m_highlight;
        public:
            PointHandleRenderer();
            
            void addPoint(const Vec3f& position);
            void addSelectedPoint(const Vec3f& position);
            void addHighlight(const Vec3f& position);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& renderContext);
            
            void clear();
        };
    }
}

#endif /* defined(__TrenchBroom__PointHandleRenderer__) */
