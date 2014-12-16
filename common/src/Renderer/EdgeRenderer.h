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

#ifndef __TrenchBroom__EdgeRenderer__
#define __TrenchBroom__EdgeRenderer__

#include "Color.h"
#include "Renderer/Renderable.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class EdgeRenderer : public Renderable {
        private:
            VertexArray m_vertexArray;
            Color m_color;
            bool m_useColor;
            bool m_prepared;
        public:
            EdgeRenderer();
            EdgeRenderer(const VertexArray& vertexArray);
            EdgeRenderer(const EdgeRenderer& other);
            EdgeRenderer& operator= (EdgeRenderer other);
            
            friend void swap(EdgeRenderer& left, EdgeRenderer& right);

            void setUseColor(bool useColor);
            void setColor(const Color& color);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& context);
        };

        // TODO: rename to RenderEdgesBase or something
        class RenderEdges : public Renderable {
        protected:
            EdgeRenderer& m_edgeRenderer;
            bool m_useColor;
            const Color& m_edgeColor;
            float m_offset;
            float m_width;
        public:
            RenderEdges(EdgeRenderer& edgeRenderer, bool useColor, const Color& edgeColor, float offset);
            virtual ~RenderEdges();
            
            void setWidth(float width);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& renderContext);
            virtual void before(RenderContext& renderContext) = 0;
            virtual void after(RenderContext& renderContext) = 0;
        };
        
        // TODO: rename to RenderEdges
        class RenderUnoccludedEdges : public RenderEdges {
        public:
            RenderUnoccludedEdges(EdgeRenderer& edgeRenderer, bool useColor, const Color& edgeColor = Color(), float offset = 0.2f);
        private:
            void before(RenderContext& renderContext);
            void after(RenderContext& renderContext);
            void doRenderEdges(RenderContext& renderContext);
        };

        // TODO: rename to RenderEdgesOnTop
        class RenderOccludedEdges : public RenderEdges {
        public:
            RenderOccludedEdges(EdgeRenderer& edgeRenderer, bool useColor, const Color& edgeColor = Color(), float offset = 0.0f);
        private:
            virtual void before(RenderContext& renderContext);
            virtual void after(RenderContext& renderContext);
            void doRenderEdges(RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__EdgeRenderer__) */
