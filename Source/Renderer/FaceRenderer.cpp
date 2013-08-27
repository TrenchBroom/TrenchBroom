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

#include "FaceRenderer.h"

#include "Model/Face.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        String FaceRenderer::AlphaBlendedTextures[] = {"clip", "hint", /*"skip",*/ "hintskip", "trigger"};

        void FaceRenderer::writeFaceData(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faceSorter) {
            const FaceCollectionMap& faceCollectionMap = faceSorter.collections();
            if (faceCollectionMap.empty())
                return;
            
            FaceCollectionMap::const_iterator it, end;
            for (it = faceCollectionMap.begin(), end = faceCollectionMap.end(); it != end; ++it) {
                Model::Texture* texture = it->first;
                TextureRenderer* textureRenderer = texture != NULL ? &textureRendererManager.renderer(texture) : NULL;
                const FaceCollection& faceCollection = it->second;
                const Model::FaceList& faces = faceCollection.polygons();
                const size_t vertexCount = 3 * faceCollection.vertexCount() - 6 * faces.size();
                VertexArray* vertexArray = new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                           Attribute::position3f(),
                                                           Attribute::normal3f(),
                                                           Attribute::texCoord02f(),
                                                           0);
                
                for (size_t i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    vertexArray->addAttributes(face->cachedVertices());
                }
                
                if (texture != NULL && alphaBlend(texture->name()))
                    m_transparentVertexArrays.push_back(TextureVertexArray(textureRenderer, vertexArray));
                else
                    m_vertexArrays.push_back(TextureVertexArray(textureRenderer, vertexArray));
            }
        }

        void FaceRenderer::render(RenderContext& context, bool grayScale, const Color* tintColor) {
            if (m_vertexArrays.empty() && m_transparentVertexArrays.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Utility::Grid& grid = context.grid();
            
            ShaderManager& shaderManager = context.shaderManager();
            ShaderProgram& faceProgram = shaderManager.shaderProgram(Shaders::FaceShader);
            
            if (faceProgram.activate()) {
                glActiveTexture(GL_TEXTURE0);
                
                const bool applyTexture = context.viewOptions().faceRenderMode() == View::ViewOptions::Textured;
                faceProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                faceProgram.setUniformVariable("Alpha", 1.0f);
                faceProgram.setUniformVariable("RenderGrid", grid.visible());
                faceProgram.setUniformVariable("GridSize", static_cast<float>(grid.actualSize()));
                faceProgram.setUniformVariable("GridAlpha", prefs.getFloat(Preferences::GridAlpha));
                faceProgram.setUniformVariable("GridCheckerboard", prefs.getBool(Preferences::GridCheckerboard));
                faceProgram.setUniformVariable("ApplyTexture", applyTexture);
                faceProgram.setUniformVariable("ApplyTinting", tintColor != NULL);
                if (tintColor != NULL)
                    faceProgram.setUniformVariable("TintColor", *tintColor);
                faceProgram.setUniformVariable("GrayScale", grayScale);
                faceProgram.setUniformVariable("CameraPosition", context.camera().position());
                faceProgram.setUniformVariable("ShadeFaces", context.viewOptions().shadeFaces() );
                faceProgram.setUniformVariable("UseFog", context.viewOptions().useFog() );
                
                renderOpaqueFaces(faceProgram, applyTexture);
                glDepthMask(GL_FALSE);
                faceProgram.setUniformVariable("Alpha", prefs.getFloat(Preferences::TransparentFaceAlpha));
                renderTransparentFaces(faceProgram, applyTexture);
                glDepthMask(GL_TRUE);

                faceProgram.deactivate();
            }
        }

        void FaceRenderer::renderOpaqueFaces(ShaderProgram& shader, const bool applyTexture) {
            renderFaces(m_vertexArrays, shader, applyTexture);
        }
        
        void FaceRenderer::renderTransparentFaces(ShaderProgram& shader, const bool applyTexture) {
            renderFaces(m_transparentVertexArrays, shader, applyTexture);
        }

        void FaceRenderer::renderFaces(const TextureVertexArrayList& vertexArrays, ShaderProgram& shader, const bool applyTexture) {
            for (size_t i = 0; i < vertexArrays.size(); i++) {
                const TextureVertexArray& textureVertexArray = vertexArrays[i];
                if (textureVertexArray.texture != NULL) {
                    textureVertexArray.texture->activate();
                    shader.setUniformVariable("ApplyTexture", applyTexture);
                    shader.setUniformVariable("FaceTexture", 0);
                    shader.setUniformVariable("Color", textureVertexArray.texture->averageColor());
                } else {
                    shader.setUniformVariable("ApplyTexture", false);
                    shader.setUniformVariable("Color", m_faceColor);
                }
                
                textureVertexArray.vertexArray->render();
                
                if (textureVertexArray.texture != NULL)
                    textureVertexArray.texture->deactivate();
            }
        }

        FaceRenderer::FaceRenderer(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faceSorter, const Color& faceColor) :
        m_faceColor(faceColor) {
            writeFaceData(vbo, textureRendererManager, faceSorter);
        }
        
        void FaceRenderer::render(RenderContext& context, bool grayScale) {
            render(context, grayScale, NULL);
        }
        
        void FaceRenderer::render(RenderContext& context, bool grayScale, const Color& tintColor) {
            render(context, grayScale, &tintColor);
        }
    }
}
