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

#include "PointHandleRenderer.h"

#include <GL/glew.h>

#include "Renderer/ApplyMatrix.h"
#include "Renderer/AttributeArray.h"
#include "Renderer/InstancedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Vec3f::List PointHandleRenderer::sphere() const {
            return Renderer::sphere(m_radius, m_iterations);
        }
        
        bool PointHandleRenderer::instancingSupported() {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            int instancingMode = prefs.getInt(Preferences::RendererInstancingMode);
            if ((instancingMode == Preferences::RendererInstancingModeForceOn) ||
                (instancingMode == Preferences::RendererInstancingModeAutodetect && GLEW_ARB_draw_instanced && GLEW_ARB_texture_float && GL_EXT_gpu_shader4))
                return true;
            return false;
        }

        PointHandleRenderer* PointHandleRenderer::create(float radius, unsigned int iterations, float scalingFactor, float maximumDistance) {
            if (instancingSupported())
               return new InstancedPointHandleRenderer(radius, iterations, scalingFactor, maximumDistance);
            return new DefaultPointHandleRenderer(radius, iterations, scalingFactor, maximumDistance);
        }

        DefaultPointHandleRenderer::DefaultPointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance) :
        PointHandleRenderer(radius, iterations, scalingFactor, maximumDistance),
        m_vertexArray(NULL) {}
        
        DefaultPointHandleRenderer::~DefaultPointHandleRenderer() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }
        
        void DefaultPointHandleRenderer::render(Vbo& vbo, RenderContext& context) {
            const Vec4f::List& positionList = positions();
            if (positionList.empty())
                return;

            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (m_vertexArray == NULL) {
                Vec3f::List vertices = sphere();
                
                unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                m_vertexArray = new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                Attribute::position3f());
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                Vec3f::List::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Vec3f& vertex = *vIt;
                    m_vertexArray->addAttribute(vertex);
                }
            }
            
            Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::PointHandleShader);
            shader.currentShader().setUniformVariable("Color", color());
            shader.currentShader().setUniformVariable("CameraPosition", context.camera().position());
            shader.currentShader().setUniformVariable("ScalingFactor", scalingFactor());
            shader.currentShader().setUniformVariable("MaximumDistance", maximumDistance());
            
            Vec4f::List::const_iterator pIt, pEnd;
            for (pIt = positionList.begin(), pEnd = positionList.end(); pIt != pEnd; ++pIt) {
                const Vec4f& position = *pIt;
                shader.currentShader().setUniformVariable("Position", position);
                m_vertexArray->render();
            }
        }

        InstancedPointHandleRenderer::InstancedPointHandleRenderer(float radius, unsigned int iterations, float scalingFactor, float maximumDistance) :
        PointHandleRenderer(radius, iterations, scalingFactor, maximumDistance),
        m_vertexArray(NULL) {}
        
        InstancedPointHandleRenderer::~InstancedPointHandleRenderer() {
            delete m_vertexArray;
            m_vertexArray = NULL;
        }

        void InstancedPointHandleRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!valid()) {
                delete m_vertexArray;
                m_vertexArray = NULL;
                
                const Vec4f::List& positionList = positions();
                
                if (!positionList.empty()) {
                    Vec3f::List vertices = sphere();
                    
                    unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
                    unsigned int instanceCount = static_cast<unsigned int>(positionList.size());
                    m_vertexArray = new InstancedVertexArray(vbo, GL_TRIANGLES, vertexCount, instanceCount,
                                                             Attribute::position3f());
                    
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    Vec3f::List::iterator it, end;
                    for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                        m_vertexArray->addAttribute(*it);
                    
                    m_vertexArray->addAttributeArray("position", positionList);
                }
                validate();
            }
            
            if (m_vertexArray != NULL) {
                Renderer::ActivateShader shader(context.shaderManager(), Renderer::Shaders::InstancedPointHandleShader);
                shader.currentShader().setUniformVariable("Color", color());
                shader.currentShader().setUniformVariable("CameraPosition", context.camera().position());
                shader.currentShader().setUniformVariable("ScalingFactor", scalingFactor());
                shader.currentShader().setUniformVariable("MaximumDistance", maximumDistance());
                m_vertexArray->render(shader.currentShader());
            }
        }
    }
}
