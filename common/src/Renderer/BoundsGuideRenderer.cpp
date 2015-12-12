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

#include "BoundsGuideRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        const FloatType BoundsGuideRenderer::SpikeLength = 512.0;

        BoundsGuideRenderer::BoundsGuideRenderer(View::MapDocumentWPtr document) :
        m_document(document) {}
        
        void BoundsGuideRenderer::setColor(const Color& color) {
            if (m_color == color)
                return;
            
            m_spikeRenderer.setColor(color);
            m_color = color;
        }
        
        void BoundsGuideRenderer::setBounds(const BBox3& bounds) {
            if (m_bounds == bounds)
                return;
            
            m_bounds = bounds;
            m_spikeRenderer.clear();
            
            View::MapDocumentSPtr document = lock(m_document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::NegX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::NegY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::NegZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::NegX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::NegY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::PosZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::NegX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::PosY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::NegZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::NegX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::PosY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::PosZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::PosX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::NegY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Min), Vec3::NegZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::PosX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::NegY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Max), Vec3::PosZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::PosX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::PosY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Min), Vec3::NegZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::PosX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::PosY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(m_bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Max), Vec3::PosZ), SpikeLength, document);
        }

        void BoundsGuideRenderer::doPrepareVertices(Vbo& vertexVbo) {
            m_spikeRenderer.prepareVertices(vertexVbo);
        }
        
        void BoundsGuideRenderer::doRender(RenderContext& renderContext) {
            m_spikeRenderer.render(renderContext);
        }
    }
}
