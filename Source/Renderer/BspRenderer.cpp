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

#include "BspRenderer.h"
#include "Model/Bsp.h"
#include "Model/Entity.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/TexturedPolygonSorter.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        void BspRenderer::buildVertexArrays() {
            typedef TexturedPolygonSorter<const Model::BspTexture, Model::BspFace*> FaceSorter;
            typedef FaceSorter::PolygonCollection FaceCollection;
            typedef FaceSorter::PolygonCollectionMap FaceCollectionMap;
            
            Model::BspModel& model = *m_bsp.models()[0];
            FaceSorter faceSorter;
            
            const Model::BspFaceList& faces = model.faces();
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::BspFace* face = faces[i];
                const Model::BspTexture& texture = face->texture();
                TextureRenderer* textureRenderer = NULL;
                
                TextureCache::iterator textureIt = m_textures.find(&texture);
                if (textureIt == m_textures.end()) {
                    textureRenderer = new TextureRenderer(texture, m_palette);
                    m_textures[&texture] = textureRenderer;
                } else {
                    textureRenderer = textureIt->second;
                }
                
                faceSorter.addPolygon(&texture, face, face->vertices().size());
            }
            
            const FaceCollectionMap& faceCollectionMap = faceSorter.collections();
            FaceCollectionMap::const_iterator it, end;
            Vec2f texCoords;
            
            m_vbo.map();
            for (it = faceCollectionMap.begin(), end = faceCollectionMap.end(); it != end; ++it) {
                const Model::BspTexture* texture = it->first;
                Renderer::TextureRenderer* textureRenderer = m_textures[texture];
                const FaceCollection& faceCollection = it->second;
                const Model::BspFaceList& collectedFaces = faceCollection.polygons();
                unsigned int vertexCount = static_cast<unsigned int>(3 * faceCollection.vertexCount() - 2 * collectedFaces.size());
                
                VertexArrayPtr vertexArray = VertexArrayPtr(new VertexArray(m_vbo, GL_TRIANGLES, vertexCount,
                                                                            VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                                                            VertexAttribute(2, GL_FLOAT, VertexAttribute::TexCoord0)));
                
                for (unsigned int i = 0; i < collectedFaces.size(); i++) {
                    Model::BspFace* face = collectedFaces[i];
                    const Vec3f::List& vertices = face->vertices();
                    for (unsigned int j = 1; j < vertices.size() - 1; j++) {
                        face->textureCoordinates(vertices[0], texCoords);
                        vertexArray->addAttribute(vertices[0]);
                        vertexArray->addAttribute(texCoords);
                        
                        face->textureCoordinates(vertices[j], texCoords);
                        vertexArray->addAttribute(vertices[j]);
                        vertexArray->addAttribute(texCoords);
                        
                        face->textureCoordinates(vertices[j + 1], texCoords);
                        vertexArray->addAttribute(vertices[j + 1]);
                        vertexArray->addAttribute(texCoords);
                    }
                }
                
                m_vertexArrays.push_back(TextureVertexArray(textureRenderer, vertexArray));
            }
            m_vbo.unmap();
        }
        
        BspRenderer::BspRenderer(const Model::Bsp& bsp, Vbo& vbo, const Palette& palette) :
        m_bsp(bsp),
        m_palette(palette),
        m_vbo(vbo) {}
        
        BspRenderer::~BspRenderer() {
            TextureCache::iterator it, end;
            for (it = m_textures.begin(), end = m_textures.end(); it != end; ++it)
                delete it->second;
            m_textures.clear();
        }

        void BspRenderer::render(ShaderProgram& shaderProgram) {
            if (m_vertexArrays.empty())
                buildVertexArrays();
            
            glActiveTexture(GL_TEXTURE0);
            for (unsigned int i = 0; i < m_vertexArrays.size(); i++) {
                TextureVertexArray& textureVertexArray = m_vertexArrays[i];
                textureVertexArray.texture->activate();
                shaderProgram.setUniformVariable("Texture", 0);
                textureVertexArray.vertexArray->render();
                textureVertexArray.texture->deactivate();
            }
        }
        
        const Vec3f& BspRenderer::center() const {
            return m_bsp.models()[0]->center();
        }
        
        const BBox& BspRenderer::bounds() const {
            return m_bsp.models()[0]->bounds();
        }
    }
}