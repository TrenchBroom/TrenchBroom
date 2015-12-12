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

#ifndef BoundsGuideRenderer_h
#define BoundsGuideRenderer_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/Renderable.h"
#include "Renderer/SpikeGuideRenderer.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class BoundsGuideRenderer : public DirectRenderable {
        private:
            static const FloatType SpikeLength;

            View::MapDocumentWPtr m_document;
            
            Color m_color;
            BBox3 m_bounds;
            SpikeGuideRenderer m_spikeRenderer;
        public:
            BoundsGuideRenderer(View::MapDocumentWPtr document);
            
            void setColor(const Color& color);
            void setBounds(const BBox3& bounds);
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* BoundsGuideRenderer_h */
