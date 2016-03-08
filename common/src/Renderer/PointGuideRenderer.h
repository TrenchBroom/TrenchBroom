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

#ifndef TrenchBroom_PointGuideRenderer
#define TrenchBroom_PointGuideRenderer

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/Renderable.h"
#include "Renderer/SpikeGuideRenderer.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class PointGuideRenderer : public DirectRenderable {
        private:
            static const FloatType SpikeLength;

            View::MapDocumentWPtr m_document;
            
            Color m_color;
            Vec3 m_position;
            SpikeGuideRenderer m_spikeRenderer;
        public:
            PointGuideRenderer(View::MapDocumentWPtr document);
            
            void setColor(const Color& color);
            void setPosition(const Vec3& position);
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* defined(TrenchBroom_PointGuideRenderer) */
