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
#include "Model/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        FaceRenderer::FaceRenderer(Vbo& vbo, const Model::BrushFace::Mesh& mesh, const Color& faceColor) :
        m_renderers(mesh.triangleSetRenderers(vbo)),
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
            
            glActiveTexture(GL_TEXTURE0);
            const bool applyTexture = true;
            shader.set("Brightness", 1.0f);
            shader.set("Alpha", 1.0f);
            shader.set("RenderGrid", true);
            shader.set("GridSize", 32);
            shader.set("GridAlpha", 0.5f);
            shader.set("GridCheckerboard", false);
            shader.set("ApplyTexture", applyTexture);
            shader.set("ApplyTinting", tintColor != NULL);
            if (tintColor != NULL)
                shader.set("TintColor", *tintColor);
            shader.set("GrayScale", grayScale);
            shader.set("CameraPosition", context.camera().position());
            shader.set("ShadeFaces", true);
            shader.set("UseFog", false);
            
            renderOpaqueFaces(shader, applyTexture);
            renderTransparentFaces(shader, applyTexture);
        }
        
        void FaceRenderer::renderOpaqueFaces(ActiveShader& shader, const bool applyTexture) {
        }
        
        void FaceRenderer::renderTransparentFaces(ActiveShader& shader, const bool applyTexture) {
            glDepthMask(GL_FALSE);
            
            glDepthMask(GL_TRUE);
        }
        
        void FaceRenderer::renderFaces(RendererMap& renderers, ActiveShader& shader, const bool applyTexture) {
            RendererMap::iterator it, end;
            for (it = renderers.begin(), end = renderers.end(); it != end; ++it) {
                Model::Texture::Ptr texture = it->first;
                VertexArrayRenderer& renderer = it->second;
                
                if (texture != NULL) {
                    texture->activate();
                    shader.set("ApplyTexture", applyTexture);
                    shader.set("FaceTexture", 0);
                    shader.set("Color", texture->averageColor());
                    renderer.render();
                    texture->deactivate();
                } else {
                    shader.set("ApplyTexture", false);
                    shader.set("Color", m_faceColor);
                    renderer.render();
                }
            }
        }
    }
}
