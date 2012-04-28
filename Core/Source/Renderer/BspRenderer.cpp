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
#include "Model/Assets/Bsp.h"
#include "Model/Map/Entity.h"
#include "Model/Preferences.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        BspRenderer::BspRenderer(Model::Assets::Bsp& bsp, Vbo& vbo, Model::Assets::Palette& palette) : m_bsp(bsp), m_vbo(vbo), m_vboBlock(NULL), m_palette(palette) {}
        
        BspRenderer::~BspRenderer() {
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
            for (TextureCache::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
                delete it->second;
            m_textures.clear();
        }
        
        void BspRenderer::render(RenderContext& context, Model::Entity& entity) {
            render(context, entity.origin(), static_cast<float>(entity.angle()));
        }
        
        void BspRenderer::render(RenderContext& context, const Vec3f& position, float angle) {
            if (m_vboBlock == NULL) {
                Model::Assets::BspModel& model = *m_bsp.models[0];
                int modelVertexCount = model.vertexCount;
                int vertexSize = 5 * sizeof(float);

                m_vboBlock = &m_vbo.allocBlock(modelVertexCount * vertexSize);
                m_vbo.map();
                int offset = 0;
                
                for (int i = 0; i < model.faces.size(); i++) {
                    Model::Assets::BspFace& face = *model.faces[i];
                    Model::Assets::BspTexture& bspTexture = *face.textureInfo->texture;
                    Model::Assets::Texture* texture = NULL;
                    TextureCache::iterator textureIt = m_textures.find(bspTexture.name);
                    if (textureIt == m_textures.end()) {
                        texture = new Model::Assets::Texture(bspTexture.name, bspTexture, m_palette);
                        m_textures[bspTexture.name] = texture;
                    } else {
                        texture = textureIt->second;
                    }

                    InfoBuffer* infoBuffer = NULL;
                    TextureVertexInfo::iterator infoIt = m_vertexInfos.find(texture);
                    if (infoIt == m_vertexInfos.end()) {
                        infoBuffer = new pair<IntBuffer*, IntBuffer*>(new IntBuffer(), new IntBuffer());
                        infoBuffer->first->reserve(0xF);
                        infoBuffer->second->reserve(0xF);
                        m_vertexInfos[texture] = infoBuffer;
                    } else {
                        infoBuffer = infoIt->second;
                    }
                    
                    infoBuffer->first->push_back(offset / vertexSize);
                    infoBuffer->second->push_back((GLsizei)face.vertices.size());
                    
                    for (int j = 0; j < face.vertices.size(); j++) {
                        const Vec3f& vertex = face.vertices[j];
                        Vec2f texCoords = face.textureCoordinates(vertex);
                        offset = m_vboBlock->writeVec(texCoords, offset);
                        offset = m_vboBlock->writeVec(vertex, offset);
                    }
                }
                m_vbo.unmap();
            }
            
            glTranslatef(position.x, position.y, position.z);
            
            if (angle != 0) {
                if (angle == -1) glRotatef(90, 1, 0, 0);
                else if (angle == -2) glRotatef(-90, 1, 0, 0);
                else glRotatef(-angle, 0, 0, 1);
            }
            
            glPolygonMode(GL_FRONT, GL_FILL);
            glEnable(GL_TEXTURE_2D);
            
            float brightness = context.preferences.brightness();
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            glColor3f(brightness / 2, brightness / 2, brightness / 2);
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glInterleavedArrays(GL_T2F_V3F, 0, (const GLvoid *)(long)m_vboBlock->address);
            
            for (TextureCache::iterator textureIt = m_textures.begin(); textureIt != m_textures.end(); ++textureIt) {
                Model::Assets::Texture* texture = textureIt->second;
                InfoBuffer* infoBuffer = m_vertexInfos[texture];
                IntBuffer* indexBuffer = infoBuffer->first;
                IntBuffer* countBuffer = infoBuffer->second;
                GLint* indexPtr = &(*indexBuffer)[0];
                GLsizei* countPtr = &(*countBuffer)[0];
                GLsizei primCount = (int)indexBuffer->size();
                
                texture->activate();
                glMultiDrawArrays(GL_POLYGON, indexPtr, countPtr, primCount);
                texture->deactivate();
            }
            
            glPopClientAttrib();
        }
        
        const Vec3f& BspRenderer::center() {
            return m_bsp.models[0]->center;
        }
        
        const BBox& BspRenderer::bounds() {
            return m_bsp.models[0]->bounds;
        }
        
        const BBox& BspRenderer::maxBounds() {
            return m_bsp.models[0]->maxBounds;
        }
    }
}