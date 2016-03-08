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

#include "SpikeGuideRenderer.h"

#include "Model/Hit.h"
#include "Model/Brush.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        SpikeGuideRenderer::SpikeGuideRenderer() :
        m_valid(false) {}
        
        void SpikeGuideRenderer::setColor(const Color& color) {
            m_color = color;
            m_valid = false;
        }
        
        void SpikeGuideRenderer::add(const Ray3& ray, const FloatType length, View::MapDocumentSPtr document) {
            Model::PickResult pickResult = Model::PickResult::byDistance(document->editorContext());
            document->pick(ray, pickResult);
            
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().minDistance(1.0).first();
            if (hit.isMatch()) {
                if (hit.distance() <= length)
                    addPoint(ray.pointAtDistance(hit.distance() - 0.01));
                addSpike(ray, Math::min(length, hit.distance()), length);
            } else {
                addSpike(ray, length, length);
            }
            m_valid = false;
        }
        
        void SpikeGuideRenderer::clear() {
            m_spikeVertices.clear();
            m_pointVertices.clear();
            m_spikeArray = VertexArray();
            m_pointArray = VertexArray();
            m_valid = true;
        }
        
        void SpikeGuideRenderer::doPrepareVertices(Vbo& vertexVbo) {
            if (!m_valid)
                validate();
            m_pointArray.prepare(vertexVbo);
            m_spikeArray.prepare(vertexVbo);
        }
        
        void SpikeGuideRenderer::doRender(RenderContext& renderContext) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
            m_spikeArray.render(GL_LINES);
            
            glAssert(glPointSize(3.0f));
            m_pointArray.render(GL_POINTS);
            glAssert(glPointSize(1.0f));
        }

        void SpikeGuideRenderer::addPoint(const Vec3& position) {
            m_pointVertices.push_back(PointVertex(position, m_color));
        }
        
        void SpikeGuideRenderer::addSpike(const Ray3& ray, const FloatType length, const FloatType maxLength) {
            const float mix = static_cast<float>(maxLength / length / 2.0);
            
            m_spikeVertices.push_back(SpikeVertex(ray.origin, m_color));
            m_spikeVertices.push_back(SpikeVertex(ray.pointAtDistance(length),
                                      Color(m_color, m_color.a() * mix)));
        }

        void SpikeGuideRenderer::validate() {
            m_pointArray = VertexArray::swap(m_pointVertices);
            m_spikeArray = VertexArray::swap(m_spikeVertices);
            m_valid = true;
        }
    }
}
