/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_SpikeGuideRenderer
#define TrenchBroom_SpikeGuideRenderer

#include "Color.h"
#include "TrenchBroom.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/ViewTypes.h"

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace Model {
        class Picker;
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class SpikeGuideRenderer : public DirectRenderable {
        private:
            Color m_color;
            
            using SpikeVertex = VertexSpecs::P3C4::Vertex;
            using PointVertex = VertexSpecs::P3C4::Vertex;
            
            SpikeVertex::List m_spikeVertices;
            PointVertex::List m_pointVertices;
            
            VertexArray m_spikeArray;
            VertexArray m_pointArray;
            
            bool m_valid;
        public:
            SpikeGuideRenderer();
            
            void setColor(const Color& color);
            void add(const vm::ray3& ray, FloatType length, View::MapDocumentSPtr document);
            void clear();
        private:
            void doPrepareVertices(Vbo& vertexVbo) override;
            void doRender(RenderContext& renderContext) override;
        private:
            void addPoint(const vm::vec3& position);
            void addSpike(const vm::ray3& ray, FloatType length, FloatType maxLength);
            
            void validate();
        };
    }
}

#endif /* defined(TrenchBroom_SpikeGuideRenderer) */
