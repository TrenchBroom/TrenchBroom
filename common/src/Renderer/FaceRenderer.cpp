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

#include "FaceRenderer.h"

#include "Renderer/GL.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        struct FaceRenderer::RenderFunc : public TextureRenderFunc {
            ActiveShader& shader;
            bool applyTexture;
            const Color& defaultColor;
            
            RenderFunc(ActiveShader& i_shader, const bool i_applyTexture, const Color& i_defaultColor) :
            shader(i_shader),
            applyTexture(i_applyTexture),
            defaultColor(i_defaultColor) {}
            
            void before(const Assets::Texture* texture) {
                if (texture != NULL) {
                    texture->activate();
                    shader.set("ApplyTexture", applyTexture);
                    shader.set("Color", texture->averageColor());
                } else {
                    shader.set("ApplyTexture", false);
                    shader.set("Color", defaultColor);
                }
            }
            
            void after(const Assets::Texture* texture) {
                if (texture != NULL)
                    texture->deactivate();
            }
        };
        
        FaceRenderer::FaceRenderer() :
        m_grayscale(false),
        m_tint(false),
        m_alpha(1.0f) {}
        
        FaceRenderer::FaceRenderer(const VertexArray& vertexArray, const IndexArray& indexArray, const TexturedIndexArrayMap& indexArrayMap, const Color& faceColor) :
        m_vertexArray(vertexArray),
        m_meshRenderer(indexArray, indexArrayMap),
        m_faceColor(faceColor),
        m_grayscale(false),
        m_tint(false),
        m_alpha(1.0f) {}

        FaceRenderer::FaceRenderer(const FaceRenderer& other) :
        m_vertexArray(other.m_vertexArray),
        m_meshRenderer(other.m_meshRenderer),
        m_faceColor(other.m_faceColor),
        m_grayscale(other.m_grayscale),
        m_tint(other.m_tint),
        m_tintColor(other.m_tintColor),
        m_alpha(other.m_alpha) {}
        
        FaceRenderer& FaceRenderer::operator=(FaceRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(FaceRenderer& left, FaceRenderer& right)  {
            using std::swap;
            swap(left.m_vertexArray, right.m_vertexArray);
            swap(left.m_meshRenderer, right.m_meshRenderer);
            swap(left.m_faceColor, right.m_faceColor);
            swap(left.m_grayscale, right.m_grayscale);
            swap(left.m_tint, right.m_tint);
            swap(left.m_tintColor, right.m_tintColor);
            swap(left.m_alpha, right.m_alpha);
        }

        void FaceRenderer::setGrayscale(const bool grayscale) {
            m_grayscale = grayscale;
        }
        
        void FaceRenderer::setTint(const bool tint) {
            m_tint = tint;
        }
        
        void FaceRenderer::setTintColor(const Color& color) {
            m_tintColor = color;
        }
        
        void FaceRenderer::setAlpha(const float alpha) {
            m_alpha = alpha;
        }

        void FaceRenderer::render(RenderBatch& renderBatch) {
            renderBatch.add(this);
        }

        void FaceRenderer::doPrepareVertices(Vbo& vertexVbo) {
            m_vertexArray.prepare(vertexVbo);
        }

        void FaceRenderer::doPrepareIndices(Vbo& indexVbo) {
            m_meshRenderer.prepare(indexVbo);
        }
        
        void FaceRenderer::doRender(RenderContext& context) {
            if (m_meshRenderer.empty())
                return;
            
            if (m_vertexArray.setup()) {
                ShaderManager& shaderManager = context.shaderManager();
                ActiveShader shader(shaderManager, Shaders::FaceShader);
                PreferenceManager& prefs = PreferenceManager::instance();
                
                const bool applyTexture = context.showTextures();
                const bool shadeFaces = context.shadeFaces();
                const bool showFog = context.showFog();
                
                glAssert(glEnable(GL_TEXTURE_2D));
                glAssert(glActiveTexture(GL_TEXTURE0));
                shader.set("Brightness", prefs.get(Preferences::Brightness));
                shader.set("RenderGrid", context.showGrid());
                shader.set("GridSize", static_cast<float>(context.gridSize()));
                shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
                shader.set("ApplyTexture", applyTexture);
                shader.set("Texture", 0);
                shader.set("ApplyTinting", m_tint);
                if (m_tint)
                    shader.set("TintColor", m_tintColor);
                shader.set("GrayScale", m_grayscale);
                shader.set("CameraPosition", context.camera().position());
                shader.set("ShadeFaces", shadeFaces);
                shader.set("ShowFog", showFog);
                shader.set("Alpha", m_alpha);
                
                RenderFunc func(shader, applyTexture, m_faceColor);
                if (m_alpha < 1.0f) {
                    glAssert(glDepthMask(GL_FALSE));
                    m_meshRenderer.render(func);
                    glAssert(glDepthMask(GL_TRUE));
                } else {
                    m_meshRenderer.render(func);
                }
                m_vertexArray.cleanup();
            }
        }
    }
}
