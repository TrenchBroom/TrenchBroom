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

#include "Model/Brush.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        const FloatType BoundsGuideRenderer::SpikeLength = 512.0;

        BoundsGuideRenderer::BoundsGuideRenderer(TextureFont& font) :
        m_showSizes(true),
        m_vbo(0xFFF),
        m_infoRenderer(font),
        m_valid(false) {}
        
        void BoundsGuideRenderer::setColor(const Color& color) {
            if (color == m_color)
                return;
            m_color = color;
            m_valid = false;
        }
        
        void BoundsGuideRenderer::setBounds(const BBox3& bounds) {
            if (bounds == m_bounds)
                return;
            m_bounds = bounds;
            m_infoRenderer.setBounds(m_bounds);
            m_valid = false;
        }
        
        void BoundsGuideRenderer::setShowSizes(const bool showSizes) {
            m_showSizes = showSizes;
        }
        
        void BoundsGuideRenderer::render(RenderContext& renderContext, View::MapDocumentSPtr document) {
            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            
            if (!m_valid)
                validate(document);
            
            ActiveShader staticColorShader(renderContext.shaderManager(), Shaders::VaryingPUniformCShader);
            staticColorShader.set("Color", m_color);
            m_boxArray.render();
            m_pointArray.render();
            
            ActiveShader dynamicColorShader(renderContext.shaderManager(), Shaders::VaryingPCShader);
            m_spikeArray.render();
            
            if (m_showSizes)
                m_infoRenderer.render(renderContext);
        }

        void BoundsGuideRenderer::validate(View::MapDocumentSPtr document) {
            validateBox();
            validateSpikes();
            validatePoints(document);

            SetVboState mapVbo(m_vbo);
            mapVbo.mapped();
            
            m_boxArray.prepare(m_vbo);
            m_spikeArray.prepare(m_vbo);
            m_pointArray.prepare(m_vbo);

            m_valid = true;
        }

        void BoundsGuideRenderer::validateBox() {
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(2 * 24);
            
            // (0,0,0) -> (1,0,0)
            vertices.push_back(Vertex(Vec3f(m_bounds.min)));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z())));
            
            // (0,0,0) -> (0,1,0)
            vertices.push_back(Vertex(Vec3f(m_bounds.min)));
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z())));
            
            // (0,0,0) -> (0,0,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.min)));
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z())));
            
            // (1,1,1) -> (0,1,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.max)));
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z())));
            
            // (1,1,1) -> (1,0,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.max)));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z())));
            
            // (1,1,1) -> (1,1,0)
            vertices.push_back(Vertex(Vec3f(m_bounds.max)));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z())));
            
            // (1,0,0) -> (1,0,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z())));
            
            // (1,0,0) -> (1,1,0)
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z())));
            
            // (0,1,0) -> (0,1,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z())));
            
            // (0,1,0) -> (1,1,0)
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z())));
            
            // (0,0,1) -> (0,1,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z())));
            
            // (0,0,1) -> (1,0,1)
            vertices.push_back(Vertex(Vec3f(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z())));
            vertices.push_back(Vertex(Vec3f(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z())));
            
            m_boxArray = VertexArray::swap(GL_LINES, vertices);
        }
        
        void addSpike(const Vec3& origin, const Vec3& direction, const FloatType length, const Color& color, VertexSpecs::P3C4::Vertex::List& vertices) {
            typedef VertexSpecs::P3C4::Vertex Vertex;
            
            vertices.push_back(Vertex(origin, color));
            vertices.push_back(Vertex(origin + length * direction, Color(color, color.a() / 2.0f)));
        }

        void BoundsGuideRenderer::validateSpikes() {
            
            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(2 * 24);

            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::NegX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::PosX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, m_color, vertices);
            
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosX, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosY, SpikeLength, m_color, vertices);
            addSpike(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, m_color, vertices);

            m_spikeArray = VertexArray::swap(GL_LINES, vertices);
        }
        
        void addPoint(const Vec3& origin, const Vec3& direction, const FloatType length, View::MapDocumentSPtr document, VertexSpecs::P3::Vertex::List& vertices) {
            typedef VertexSpecs::P3::Vertex Vertex;

            const Hits hits = document->pick(Ray3(origin, direction));
            const Hits::List brushHits = hits.filter(Model::Brush::BrushHit);

            Hits::List::const_iterator it, end;
            for (it = brushHits.begin(), end = brushHits.end(); it != end; ++it) {
                const Hit& hit = *it;
                if (Math::abs(hit.distance()) < length)
                    vertices.push_back(Vertex(hit.hitPoint() - direction / 10.0f));
            }
        }
        
        void BoundsGuideRenderer::validatePoints(View::MapDocumentSPtr document) {
            typedef VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices;
            
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::NegX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::PosX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::NegY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::PosY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z()), Vec3::NegZ, SpikeLength, document, vertices);
            
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosX, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosY, SpikeLength, document, vertices);
            addPoint(Vec3(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z()), Vec3::PosZ, SpikeLength, document, vertices);
            
            m_pointArray = VertexArray::swap(GL_POINTS, vertices);
        }
    }
}
