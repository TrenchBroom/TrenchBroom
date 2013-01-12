/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BoxGuideRenderer.h"

#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Renderer {
        void BoxGuideRenderer::addSpike(const Vec3f& startPoint, const Vec3f& direction, Vec3f::List& hitPoints) {
            float maxLength = 512.0f;
            Vec3f endPoint = startPoint + maxLength * direction;
            
            Model::PickResult* result = m_picker.pick(Ray(startPoint, direction));
            Model::HitList hits = result->hits(Model::HitType::FaceHit, m_filter);
            Model::HitList::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                Model::Hit& hit = **it;
                if (std::abs((hit.hitPoint() - startPoint).dot(direction)) < maxLength)
                    hitPoints.push_back(hit.hitPoint() - direction / 10.0f); // nudge the point a little bit away to make it visible
            }
            
            m_spikeArray->addAttribute(startPoint);
            m_spikeArray->addAttribute(m_color);
            m_spikeArray->addAttribute(endPoint);
            m_spikeArray->addAttribute(Vec4f(m_color, m_color.w / 2.0f));

            delete result;
        }
        
        BoxGuideRenderer::BoxGuideRenderer(const BBox& bounds, Model::Picker& picker, Model::Filter& defaultFilter, Text::StringManager& stringManager) :
        m_infoRenderer(BoxInfoRenderer(bounds, stringManager)),
        m_color(Color(1.0f, 1.0f, 0.0f, 1.0f)),
        m_bounds(bounds),
        m_picker(picker),
        m_filter(Model::VisibleFilter(defaultFilter)),
        m_boxArray(NULL),
        m_spikeArray(NULL),
        m_pointArray(NULL),
        m_showSizes(false),
        m_valid(false){}
        
        BoxGuideRenderer::~BoxGuideRenderer() {
            delete m_boxArray;
            m_boxArray = NULL;
            delete m_spikeArray;
            m_spikeArray = NULL;
            delete m_pointArray;
            m_pointArray = NULL;
        }

        void BoxGuideRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                delete m_boxArray;
                m_boxArray = NULL;
                delete m_spikeArray;
                m_spikeArray = NULL;
                delete m_pointArray;
                m_pointArray = NULL;
            }
            
            if (m_boxArray == NULL) {
                assert(m_spikeArray == NULL);
                assert(m_pointArray == NULL);
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_boxArray = new VertexArray(vbo, GL_LINES, 24, Attribute::position3f(), Attribute::color4f());
                m_spikeArray = new VertexArray(vbo, GL_LINES, 48, Attribute::position3f(), Attribute::color4f());
                
                
                // (0,0,0) -> (1,0,0)
                m_boxArray->addAttribute(m_bounds.min);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);

                // (0,0,0) -> (0,1,0)
                m_boxArray->addAttribute(m_bounds.min);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);

                // (0,0,0) -> (0,0,1)
                m_boxArray->addAttribute(m_bounds.min);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                
                // (1,1,1) -> (0,1,1)
                m_boxArray->addAttribute(m_bounds.max);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);

                // (1,1,1) -> (1,0,1)
                m_boxArray->addAttribute(m_bounds.max);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);

                // (1,1,1) -> (1,1,0)
                m_boxArray->addAttribute(m_bounds.max);
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);

                // (1,0,0) -> (1,0,1)
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);

                // (1,0,0) -> (1,1,0)
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);

                // (0,1,0) -> (0,1,1)
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                
                // (0,1,0) -> (1,1,0)
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z));
                m_boxArray->addAttribute(m_color);

                // (0,0,1) -> (0,1,1)
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                
                // (0,0,1) -> (1,0,1)
                m_boxArray->addAttribute(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                m_boxArray->addAttribute(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z));
                m_boxArray->addAttribute(m_color);
                
                Vec3f::List hitPoints;
                
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.min.z), Vec3f::NegX, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.min.z), Vec3f::NegY, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.min.z), Vec3f::NegZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z), Vec3f::NegX, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z), Vec3f::NegY, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.min.y, m_bounds.max.z), Vec3f::PosZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z), Vec3f::NegX, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z), Vec3f::PosY, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.min.z), Vec3f::NegZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z), Vec3f::NegX, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z), Vec3f::PosY, hitPoints);
                addSpike(Vec3f(m_bounds.min.x, m_bounds.max.y, m_bounds.max.z), Vec3f::PosZ, hitPoints);

                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z), Vec3f::PosX, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z), Vec3f::NegY, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.min.z), Vec3f::NegZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z), Vec3f::PosX, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z), Vec3f::NegY, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.min.y, m_bounds.max.z), Vec3f::PosZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z), Vec3f::PosX, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z), Vec3f::PosY, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.min.z), Vec3f::NegZ, hitPoints);
                
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.max.z), Vec3f::PosX, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.max.z), Vec3f::PosY, hitPoints);
                addSpike(Vec3f(m_bounds.max.x, m_bounds.max.y, m_bounds.max.z), Vec3f::PosZ, hitPoints);

                if (!hitPoints.empty()) {
                    m_pointArray = new VertexArray(vbo, GL_POINTS, static_cast<unsigned int>(hitPoints.size()), Attribute::position3f());
                    Vec3f::List::const_iterator it, end;
                    for (it = hitPoints.begin(), end = hitPoints.end(); it != end; ++it)
                        m_pointArray->addAttribute(*it);
                }
            }
            
            assert(m_boxArray != NULL && m_spikeArray != NULL);
            
            glDisable(GL_DEPTH_TEST);
            ActivateShader lineShader(context.shaderManager(), Shaders::ColoredEdgeShader);
            m_boxArray->render();
            glEnable(GL_DEPTH_TEST);
            m_spikeArray->render();
            
            if (m_pointArray != NULL) {
                ActivateShader pointShader(context.shaderManager(), Shaders::EdgeShader);
                pointShader.currentShader().setUniformVariable("Color", Vec4f(m_color, 1.0f));
                glEnable(GL_POINT_SMOOTH);
                glPointSize(3.0f);
                m_pointArray->render();
                glPointSize(1.0f);
            }
            
            if (m_showSizes)
                m_infoRenderer.render(vbo, context);
        }
    }
}
