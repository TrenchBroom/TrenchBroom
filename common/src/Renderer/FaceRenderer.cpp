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
#include "Renderer/Shaders.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        struct TextureMeshFunc : public TexturedTriangleMeshRenderer::MeshFuncBase {
            ActiveShader& shader;
            bool applyTexture;
            const Color& defaultColor;
            
            TextureMeshFunc(ActiveShader& i_shader, const bool i_applyTexture, const Color& i_defaultColor) :
            shader(i_shader),
            applyTexture(i_applyTexture),
            defaultColor(i_defaultColor) {}
            
            void before(const Assets::Texture* const & texture) const {
                if (texture != NULL) {
                    texture->activate();
                    shader.set("ApplyTexture", applyTexture);
                    shader.set("Color", texture->averageColor());
                } else {
                    shader.set("ApplyTexture", false);
                    shader.set("Color", defaultColor);
                }
            }
            
            void after(const Assets::Texture* const & texture) const {
                if (texture != NULL)
                    texture->deactivate();
            }
        };
        
        FaceRenderer::FaceRenderer() :
        m_grayscale(false),
        m_tint(false),
        m_alpha(1.0f),
        m_prepared(true) {}
        
        FaceRenderer::FaceRenderer(Model::BrushFace::Mesh& mesh, const Color& faceColor) :
        m_meshRenderer(mesh),
        m_faceColor(faceColor),
        m_grayscale(false),
        m_tint(false),
        m_alpha(1.0f),
        m_prepared(false) {}

        FaceRenderer::FaceRenderer(const FaceRenderer& other) :
        m_meshRenderer(other.m_meshRenderer),
        m_faceColor(other.m_faceColor),
        m_grayscale(other.m_grayscale),
        m_tint(other.m_tint),
        m_tintColor(other.m_tintColor),
        m_alpha(other.m_alpha),
        m_prepared(other.m_prepared) {}
        
        FaceRenderer& FaceRenderer::operator= (FaceRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(FaceRenderer& left, FaceRenderer& right)  {
            using std::swap;
            swap(left.m_meshRenderer, right.m_meshRenderer);
            swap(left.m_faceColor, right.m_faceColor);
            swap(left.m_prepared, right.m_prepared);
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

        void FaceRenderer::doPrepare(Vbo& vbo) {
            if (!m_prepared) {
                m_meshRenderer.prepare(vbo);
                m_prepared = true;
            }
        }

        void FaceRenderer::doRender(RenderContext& context) {
            assert(m_prepared);
            
            if (m_meshRenderer.empty())
                return;
            
            ShaderManager& shaderManager = context.shaderManager();
            ActiveShader shader(shaderManager, Shaders::FaceShader);
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const bool applyTexture = context.showTextures();
            const bool shadeFaces = context.shadeFaces();
            const bool showFog = context.showFog();

            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
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
            if (m_alpha < 1.0f) {
                glDepthMask(GL_FALSE);
                m_meshRenderer.render(TextureMeshFunc(shader, applyTexture, m_faceColor));
                glDepthMask(GL_TRUE);
            } else {
                m_meshRenderer.render(TextureMeshFunc(shader, applyTexture, m_faceColor));
            }
        }
    }
}
