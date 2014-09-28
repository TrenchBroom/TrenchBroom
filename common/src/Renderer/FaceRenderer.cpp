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
#include "Renderer/RenderConfig.h"
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
        
        FaceRenderer::Config::Config() :
        alpha(1.0f),
        grayscale(false),
        tinted(false) {}
        
        FaceRenderer::FaceRenderer() :
        m_prepared(true) {}
        
        FaceRenderer::FaceRenderer(Model::BrushFace::Mesh& mesh, const Color& faceColor) :
        m_vbo(new Vbo(mesh.size())),
        m_meshRenderer(mesh),
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

        void FaceRenderer::render(RenderContext& context, const Config& config) {
            render(context, config.alpha, config.grayscale, config.tinted ? &config.tintColor : NULL);
        }

        void FaceRenderer::render(RenderContext& context, const float alpha, const bool grayscale, const Color* tintColor) {
            if (m_meshRenderer.empty())
                return;
            
            SetVboState setVboState(*m_vbo);
            setVboState.active();
            if (!m_prepared)
                prepare();
            
            ShaderManager& shaderManager = context.shaderManager();
            ActiveShader shader(shaderManager, Shaders::FaceShader);
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const bool applyTexture = context.renderConfig().showTextures();
            const bool shadeFaces = context.renderConfig().shadeFaces();
            const bool useFog = context.renderConfig().useFog();

            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("RenderGrid", context.showGrid());
            shader.set("GridSize", static_cast<float>(context.gridSize()));
            shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
            shader.set("ApplyTexture", applyTexture);
            shader.set("Texture", 0);
            shader.set("ApplyTinting", tintColor != NULL);
            if (tintColor != NULL)
                shader.set("TintColor", *tintColor);
            shader.set("GrayScale", grayscale);
            shader.set("CameraPosition", context.camera().position());
            shader.set("ShadeFaces", shadeFaces);
            shader.set("UseFog", useFog);
            
            shader.set("Alpha", alpha);
            if (alpha < 1.0f) {
                glDepthMask(GL_FALSE);
                m_meshRenderer.render(SetShaderParms(shader, applyTexture, m_faceColor));
                glDepthMask(GL_TRUE);
            } else {
                m_meshRenderer.render(SetShaderParms(shader, applyTexture, m_faceColor));
            }
        }

        void FaceRenderer::prepare() {
            assert(!m_prepared);
            SetVboState setVboState(*m_vbo);
            setVboState.mapped();
            m_meshRenderer.prepare(*m_vbo);
            m_prepared = true;
        }
    }
}
