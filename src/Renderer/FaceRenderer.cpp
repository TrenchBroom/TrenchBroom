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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FaceRenderer.h"

#include "GL/GL.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        FaceRenderer::FaceRenderer() {}
        
        FaceRenderer::FaceRenderer(Vbo& vbo, const Model::BrushFace::Mesh& mesh, const Color& faceColor) :
        m_arrays(mesh.triangleSetArrays(vbo)),
        m_faceColor(faceColor) {}

        void FaceRenderer::render(RenderContext& context, const bool grayScale) {
            render(context, grayScale, NULL);
        }
        
        void FaceRenderer::render(RenderContext& context, const bool grayScale, const Color& tintColor) {
            render(context, grayScale, &tintColor);
        }

        void FaceRenderer::render(RenderContext& context, bool grayScale, const Color* tintColor) {
            ShaderManager& shaderManager = context.shaderManager();
            ActiveShader shader(shaderManager, Shaders::FaceShader);
            PreferenceManager& prefs = PreferenceManager::instance();
            
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            const bool applyTexture = true;
            shader.set("Brightness", prefs.getFloat(Preferences::Brightness));
            shader.set("Alpha", 1.0f);
            shader.set("RenderGrid", true);
            shader.set("GridSize", 32.0f);
            shader.set("GridAlpha", prefs.getFloat(Preferences::GridAlpha));
            shader.set("GridCheckerboard", prefs.getBool(Preferences::GridCheckerboard));
            shader.set("ApplyTexture", applyTexture);
            shader.set("FaceTexture", 0);
            shader.set("ApplyTinting", tintColor != NULL);
            if (tintColor != NULL)
                shader.set("TintColor", *tintColor);
            shader.set("GrayScale", grayScale);
            shader.set("CameraPosition", context.camera().position());
            shader.set("ShadeFaces", prefs.getBool(Preferences::ShadeFaces));
            shader.set("UseFog", prefs.getBool(Preferences::UseFog));
            
            renderOpaqueFaces(shader, applyTexture);
            renderTransparentFaces(shader, applyTexture);
        }
        
        void FaceRenderer::renderOpaqueFaces(ActiveShader& shader, const bool applyTexture) {
            renderFaces(m_arrays, shader, applyTexture);
        }
        
        void FaceRenderer::renderTransparentFaces(ActiveShader& shader, const bool applyTexture) {
            /*
            glDepthMask(GL_FALSE);
            glDepthMask(GL_TRUE);
             */
        }
        
        void FaceRenderer::renderFaces(VertexArrayMap& arrays, ActiveShader& shader, const bool applyTexture) {
            VertexArrayMap::iterator it, end;
            for (it = arrays.begin(), end = arrays.end(); it != end; ++it) {
                Model::TexturePtr texture = it->first;
                VertexArray& array = it->second;
                
                if (texture != NULL) {
                    texture->activate();
                    shader.set("ApplyTexture", applyTexture);
                    shader.set("Color", texture->averageColor());
                    array.render();
                    texture->deactivate();
                } else {
                    shader.set("ApplyTexture", false);
                    shader.set("Color", m_faceColor);
                    array.render();
                }
            }
        }
    }
}
