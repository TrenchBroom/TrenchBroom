/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "Assets/FaceTexture.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        struct SetShaderParms {
            ActiveShader& shader;
            bool applyTexture;
            const Color& defaultColor;
            
            SetShaderParms(ActiveShader& i_shader, const bool i_applyTexture, const Color& i_defaultColor) :
            shader(i_shader),
            applyTexture(i_applyTexture),
            defaultColor(i_defaultColor) {}
            
            void operator()(const Assets::Texture* texture) const {
                if (texture != NULL) {
                    shader.set("ApplyTexture", applyTexture);
                    shader.set("Color", texture->averageColor());
                } else {
                    shader.set("ApplyTexture", false);
                    shader.set("Color", defaultColor);
                }
            }
        };
        
        FaceRenderer::FaceRenderer() :
        m_prepared(true) {}
        
        FaceRenderer::FaceRenderer(const Model::BrushFace::Mesh& mesh, const Color& faceColor) :
        m_vbo(new Vbo(mesh.size())),
        m_meshRenderer(*m_vbo, mesh),
        m_faceColor(faceColor),
        m_prepared(false) {}

        FaceRenderer::FaceRenderer(const FaceRenderer& other) :
        m_vbo(other.m_vbo),
        m_meshRenderer(other.m_meshRenderer),
        m_faceColor(other.m_faceColor),
        m_prepared(other.m_prepared) {}
        
        FaceRenderer& FaceRenderer::operator= (FaceRenderer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(FaceRenderer& left, FaceRenderer& right)  {
            using std::swap;
            swap(left.m_vbo, right.m_vbo);
            swap(left.m_meshRenderer, right.m_meshRenderer);
            swap(left.m_faceColor, right.m_faceColor);
            swap(left.m_prepared, right.m_prepared);
        }

        void FaceRenderer::render(RenderContext& context, const bool grayscale) {
            render(context, grayscale, NULL);
        }
        
        void FaceRenderer::render(RenderContext& context, const bool grayscale, const Color& tintColor) {
            render(context, grayscale, &tintColor);
        }

        void FaceRenderer::render(RenderContext& context, bool grayscale, const Color* tintColor) {
            if (m_meshRenderer.empty())
                return;
            
            SetVboState setVboState(*m_vbo);
            setVboState.active();
            if (!m_prepared)
                prepare();
            
            ShaderManager& shaderManager = context.shaderManager();
            ActiveShader shader(shaderManager, Shaders::FaceShader);
            PreferenceManager& prefs = PreferenceManager::instance();
            
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            const bool applyTexture = true;
            shader.set("Brightness", prefs.getFloat(Preferences::Brightness));
            shader.set("Alpha", 1.0f);
            shader.set("RenderGrid", context.gridVisible());
            shader.set("GridSize", static_cast<float>(context.gridSize()));
            shader.set("GridAlpha", prefs.getFloat(Preferences::GridAlpha));
            shader.set("GridCheckerboard", prefs.getBool(Preferences::GridCheckerboard));
            shader.set("ApplyTexture", applyTexture);
            shader.set("FaceTexture", 0);
            shader.set("ApplyTinting", tintColor != NULL);
            if (tintColor != NULL)
                shader.set("TintColor", *tintColor);
            shader.set("GrayScale", grayscale);
            shader.set("CameraPosition", context.camera().position());
            shader.set("ShadeFaces", prefs.getBool(Preferences::ShadeFaces));
            shader.set("UseFog", prefs.getBool(Preferences::UseFog));
            
            renderOpaqueFaces(shader, applyTexture);
            renderTransparentFaces(shader, applyTexture);
        }
        
        void FaceRenderer::renderOpaqueFaces(ActiveShader& shader, const bool applyTexture) {
            m_meshRenderer.render(SetShaderParms(shader, applyTexture, m_faceColor));
        }
        
        void FaceRenderer::renderTransparentFaces(ActiveShader& shader, const bool applyTexture) {
            /*
            glDepthMask(GL_FALSE);
            glDepthMask(GL_TRUE);
             */
        }

        void FaceRenderer::prepare() {
            assert(!m_prepared);
            SetVboState setVboState(*m_vbo);
            setVboState.mapped();
            m_meshRenderer.prepare();
            m_prepared = true;
        }
    }
}
