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

#ifndef TrenchBroom_PointHandleRenderer
#define TrenchBroom_PointHandleRenderer

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class Vbo;
        
        class PointHandleRenderer : public DirectRenderable {
        private:
            typedef std::map<Color, Vec3f::List> HandleMap;
            
            HandleMap m_pointHandles;
            HandleMap m_highlights;

            Circle m_handle;
            Circle m_highlight;
        public:
            PointHandleRenderer();
            
            void addPoint(const Color& color, const Vec3f& position);
            void addHighlight(const Color& color, const Vec3f& position);
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
            void renderHandles(RenderContext& renderContext, const HandleMap& map, Circle& circle);
            
            void clear();
        };
    }
}

#endif /* defined(TrenchBroom_PointHandleRenderer) */
