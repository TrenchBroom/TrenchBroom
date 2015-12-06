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

#ifndef TrenchBroom_Compass
#define TrenchBroom_Compass

#include "Color.h"
#include "VecMath.h"
#include "Renderer/IndexRangeRenderer.h"
#include "Renderer/Renderable.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
        class Vbo;
        
        class Compass : public DirectRenderable {
        private:
            static const size_t m_segments;
            static const float m_shaftLength;
            static const float m_shaftRadius;
            static const float m_headLength;
            static const float m_headRadius;

            IndexRangeRenderer m_arrowRenderer;
            IndexRangeRenderer m_backgroundRenderer;
            IndexRangeRenderer m_backgroundOutlineRenderer;
            bool m_prepared;
        public:
            Compass();
            virtual ~Compass();
            
            void render(RenderBatch& renderBatch);
        private: // implement Renderable interface
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
        private:
            void makeArrows();
            void makeBackground();
            
            Mat4x4f cameraRotationMatrix(const Camera& camera) const;
        protected:
            void renderBackground(RenderContext& renderContext);
            void renderSolidAxis(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color);
            void renderAxisOutline(RenderContext& renderContext, const Mat4x4f& transformation, const Color& color);
            void renderAxis(RenderContext& renderContext, const Mat4x4f& transformation);
        private:
            virtual void doRenderCompass(RenderContext& renderContext, const Mat4x4f& cameraTransformation) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Compass) */
