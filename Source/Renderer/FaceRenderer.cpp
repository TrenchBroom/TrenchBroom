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

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        void FaceRenderer::writeFaceData(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faces) {
            const FaceCollectionMap& faceCollectionMap = faces.collections();
            if (faceCollectionMap.empty())
                return;
            
            FaceCollectionMap::const_iterator it, end;
            for (it = faceCollectionMap.begin(), end = faceCollectionMap.end(); it != end; ++it) {
                Model::Texture* texture = it->first;
                TextureRenderer* textureRenderer = texture != NULL ? &textureRendererManager.renderer(texture) : NULL;
                const FaceCollection& faceCollection = it->second;
                const Model::FaceList& faces = faceCollection.polygons();
                unsigned int vertexCount = static_cast<unsigned int>(3 * faceCollection.vertexCount() - 2 * faces.size());
                VertexArrayPtr vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_TRIANGLES, vertexCount,
                                                                            Attribute::position3f(),
                                                                            Attribute::normal3f(),
                                                                            Attribute::texCoord02f()));
                
                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    const Model::VertexList& vertices = face->vertices();
                    const Vec2f::List& texCoords = face->texCoords();
                    
                    for (unsigned int j = 1; j < vertices.size() - 1; j++) {
                        vertexArray->addAttribute(vertices[0]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[0]);
                        vertexArray->addAttribute(vertices[j]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[j]);
                        vertexArray->addAttribute(vertices[j + 1]->position);
                        vertexArray->addAttribute(face->boundary().normal);
                        vertexArray->addAttribute(texCoords[j + 1]);
                    }
                }
                
                m_vertexArrays.push_back(TextureVertexArray(textureRenderer, vertexArray));
            }
        }

        void FaceRenderer::render(RenderContext& context, bool grayScale, const Color* tintColor) {
            if (m_vertexArrays.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Utility::Grid& grid = context.grid();
            
            ShaderManager& shaderManager = context.shaderManager();
            ShaderProgram& faceProgram = shaderManager.shaderProgram(Shaders::FaceShader);
            
            if (faceProgram.activate()) {
                glActiveTexture(GL_TEXTURE0);
                
                const bool applyTexture = context.viewOptions().faceRenderMode() == View::ViewOptions::Textured;
                faceProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                faceProgram.setUniformVariable("RenderGrid", grid.visible());
                faceProgram.setUniformVariable("GridSize", static_cast<float>(grid.actualSize()));
                faceProgram.setUniformVariable("GridColor", prefs.getColor(Preferences::GridColor));
                faceProgram.setUniformVariable("ApplyTexture", applyTexture);
                faceProgram.setUniformVariable("ApplyTinting", tintColor != NULL);
                if (tintColor != NULL)
                    faceProgram.setUniformVariable("TintColor", *tintColor);
                faceProgram.setUniformVariable("GrayScale", grayScale);
                
                for (unsigned int i = 0; i < m_vertexArrays.size(); i++) {
                    TextureVertexArray& textureVertexArray = m_vertexArrays[i];
                    if (textureVertexArray.texture != NULL) {
                        textureVertexArray.texture->activate();
                        faceProgram.setUniformVariable("ApplyTexture", applyTexture);
                        faceProgram.setUniformVariable("FaceTexture", 0);
                        faceProgram.setUniformVariable("Color", textureVertexArray.texture->averageColor());
                    } else {
                        faceProgram.setUniformVariable("ApplyTexture", false);
                        faceProgram.setUniformVariable("Color", m_faceColor);
                    }
                    
                    textureVertexArray.vertexArray->render();
                    
                    if (textureVertexArray.texture != NULL)
                        textureVertexArray.texture->deactivate();
                }
                faceProgram.deactivate();
            }
        }

        FaceRenderer::FaceRenderer(Vbo& vbo, TextureRendererManager& textureRendererManager, const Sorter& faces, const Color& faceColor) :
        m_faceColor(faceColor) {
            writeFaceData(vbo, textureRendererManager, faces);
        }
        
        void FaceRenderer::render(RenderContext& context, bool grayScale) {
            render(context, grayScale, NULL);
        }
        
        void FaceRenderer::render(RenderContext& context, bool grayScale, const Color& tintColor) {
            render(context, grayScale, &tintColor);
        }
    }
}