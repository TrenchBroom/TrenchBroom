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

#include "PointGuideRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        const FloatType PointGuideRenderer::SpikeLength = 512.0;

        PointGuideRenderer::PointGuideRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_vbo(0xFF),
        m_spikeRenderer(m_vbo) {}
        
        void PointGuideRenderer::setColor(const Color& color) {
            m_spikeRenderer.setColor(color);
        }
        
        void PointGuideRenderer::setPosition(const Vec3& position) {
            if (position == m_position)
                return;
            
            m_spikeRenderer.clear();

            View::MapDocumentSPtr document = lock(m_document);
            m_spikeRenderer.add(Ray3(position, Vec3::PosX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(position, Vec3::NegX), SpikeLength, document);
            m_spikeRenderer.add(Ray3(position, Vec3::PosY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(position, Vec3::NegY), SpikeLength, document);
            m_spikeRenderer.add(Ray3(position, Vec3::PosZ), SpikeLength, document);
            m_spikeRenderer.add(Ray3(position, Vec3::NegZ), SpikeLength, document);
            
            m_position = position;
        }
        
        void PointGuideRenderer::render(RenderContext& renderContext) {
            m_spikeRenderer.render(renderContext);
        }
    }
}
