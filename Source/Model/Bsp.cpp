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

#include "Bsp.h"

#include <cmath>
#include <cstring>
#include <numeric>

namespace TrenchBroom {
    namespace Model {
        BspTexture::BspTexture(const String& name, const unsigned char* image, unsigned int width, unsigned int height) :
        m_name(name),
        m_image(image),
        m_width(width),
        m_height(height) {}
        
        BspTexture::~BspTexture() {
            delete[] m_image;
            m_image = NULL;
        }
        
        BspFace::BspFace(BspTextureInfo* textureInfo, const Vec3f::List& vertices) :
        m_textureInfo(textureInfo),
        m_vertices(vertices) {
            m_bounds.min = m_bounds.max = m_vertices.front();
            
            for (unsigned int i = 1; i < m_vertices.size(); i++)
                m_bounds.mergeWith(m_vertices[i]);
        }
        
        BspModel::BspModel(const BspFaceList& faces, unsigned int vertexCount, const Vec3f& center, const BBox& bounds) :
        m_faces(faces),
        m_vertexCount(vertexCount),
        m_center(center),
        m_bounds(bounds) {}
        
        BspModel::~BspModel() {
            while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
        }
        
        void Bsp::readTextures(IO::PakStream& stream, unsigned int count) {
            int32_t offset;
            uint32_t width, height, mip0Offset;
            char textureName[BspLayout::TextureNameLength];
            unsigned char* mip0;
            std::streamoff base;
            
            base = stream->tellg();
            
            for (unsigned int i = 0; i < count; i++) {
                stream->seekg(base + (i + 1) * sizeof(int32_t), std::ios::beg);
                stream->read(reinterpret_cast<char *>(&offset), sizeof(int32_t));
                stream->seekg(base + offset, std::ios::beg);
                stream->read(textureName, BspLayout::TextureNameLength);
                stream->read(reinterpret_cast<char *>(&width), sizeof(uint32_t));
                stream->read(reinterpret_cast<char *>(&height), sizeof(uint32_t));
                stream->read(reinterpret_cast<char *>(&mip0Offset), sizeof(uint32_t));
                
                mip0 = new unsigned char[width * height];
                stream->seekg(base + offset + mip0Offset, std::ios::beg);
                stream->read(reinterpret_cast<char *>(mip0), width * height);
                
                BspTexture* texture = new BspTexture(textureName, mip0, width, height);
                m_textures[i] = texture;
            }
        }
        
        void Bsp::readTextureInfos(IO::PakStream& stream, unsigned int count, std::vector<BspTexture*>& textures) {
            uint32_t textureIndex;
            
            for (unsigned int i = 0; i < count; i++) {
                BspTextureInfo* textureInfo = new BspTextureInfo();
                stream->read(reinterpret_cast<char *>(&textureInfo->sAxis), 3 * sizeof(float));
                stream->read(reinterpret_cast<char *>(&textureInfo->sOffset), sizeof(float));
                stream->read(reinterpret_cast<char *>(&textureInfo->tAxis), 3 * sizeof(float));
                stream->read(reinterpret_cast<char *>(&textureInfo->tOffset), sizeof(float));
                stream->read(reinterpret_cast<char *>(&textureIndex), sizeof(uint32_t));
                textureInfo->texture = textures[textureIndex];
                stream->seekg(BspLayout::TexInfoRest, std::ios::cur);
                
                m_textureInfos[i] = textureInfo;
            }
        }
        
        void Bsp::readVertices(IO::PakStream& stream, unsigned int count, Vec3f::List& vertices) {
            Vec3f vertex;
            for (unsigned int i = 0; i < count; i++) {
                stream->read(reinterpret_cast<char *>(&vertex), sizeof(Vec3f));
                vertices.push_back(vertex);
            }
        }
        
        void Bsp::readEdges(IO::PakStream& stream, unsigned int count, BspEdgeInfoList& edges) {
            BspEdgeInfo edge;
            for (unsigned int i = 0; i < count; i++) {
                stream->read(reinterpret_cast<char *>(&edge), sizeof(BspEdgeInfo));
                edges.push_back(edge);
            }
        }
        
        void Bsp::readFaces(IO::PakStream& stream, unsigned int count, BspFaceInfoList& faces) {
            BspFaceInfo face;
            for (unsigned int i = 0; i < count; i++) {
                stream->seekg(BspLayout::FaceEdgeIndex, std::ios::cur);
                stream->read(reinterpret_cast<char *>(&face.edgeIndex), sizeof(int32_t));
                stream->read(reinterpret_cast<char *>(&face.edgeCount), sizeof(uint16_t));
                stream->read(reinterpret_cast<char *>(&face.textureInfoIndex), sizeof(uint16_t));
                stream->seekg(BspLayout::FaceRest, std::ios::cur);
                
                faces.push_back(face);
            }
        }
        
        void Bsp::readFaceEdges(IO::PakStream& stream, unsigned int count, int32_t* indices) {
            stream->read(reinterpret_cast<char *>(indices), count * sizeof(int32_t));
        }
        
        Bsp::Bsp(const String& name, IO::PakStream stream) :
        m_name(name) {
            int32_t version;
            stream->read(reinterpret_cast<char *>(&version), sizeof(int32_t));
            
            int32_t textureAddr, textureCount;
            stream->seekg(BspLayout::DirTexturesAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&textureAddr), sizeof(int32_t));
            
            stream->seekg(textureAddr, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&textureCount), sizeof(int32_t));
            stream->seekg(-static_cast<int>(sizeof(int32_t)), std::ios::cur);
            m_textures.resize(textureCount);
            readTextures(stream, textureCount);
            
            int32_t texInfosAddr, texInfosLength;
            unsigned int texInfoCount;
            stream->seekg(BspLayout::DirTexInfosAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&texInfosAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&texInfosLength), sizeof(int32_t));
            texInfoCount = texInfosLength / BspLayout::TexInfoSize;
            m_textureInfos.resize(texInfoCount);
            stream->seekg(texInfosAddr, std::ios::beg);
            readTextureInfos(stream, texInfoCount, m_textures);
            
            int32_t verticesAddr, verticesLength;
            unsigned int vertexCount;
            stream->seekg(BspLayout::DirVerticesAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&verticesAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&verticesLength), sizeof(int32_t));
            vertexCount = verticesLength / sizeof(Vec3f);
            
            Vec3f::List vertices;
            stream->seekg(verticesAddr, std::ios::beg);
            readVertices(stream, vertexCount, vertices);
            
            int32_t edgesAddr, edgesLength;
            unsigned int edgeCount;
            stream->seekg(BspLayout::DirEdgesAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&edgesAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&edgesLength), sizeof(int32_t));
            edgeCount = edgesLength / sizeof(BspEdgeInfo);
            
            BspEdgeInfoList edges;
            stream->seekg(edgesAddr, std::ios::beg);
            readEdges(stream, edgeCount, edges);
            
            int32_t facesAddr, facesLength;
            unsigned int faceCount;
            stream->seekg(BspLayout::DirFacesAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&facesAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&facesLength), sizeof(int32_t));
            faceCount = facesLength / BspLayout::FaceSize;
            
            BspFaceInfoList faces;
            stream->seekg(facesAddr, std::ios::beg);
            readFaces(stream, faceCount, faces);
            
            int32_t faceEdgesAddr, faceEdgesLength;
            unsigned int faceEdgesCount;
            stream->seekg(BspLayout::DirFaceEdgesAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&faceEdgesAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&faceEdgesLength), sizeof(int32_t));
            faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;
            
            int32_t* faceEdges = new int32_t[faceEdgesCount];
            stream->seekg(faceEdgesAddr, std::ios::beg);
            readFaceEdges(stream, faceEdgesCount, faceEdges);
            
            int32_t modelsAddr, modelsLength;
            unsigned int modelCount;
            stream->seekg(BspLayout::DirModelAddress, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&modelsAddr), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&modelsLength), sizeof(int32_t));
            modelCount = modelsLength / BspLayout::ModelSize;
            
            bool* vertexMark = new bool[vertexCount];
            memset(vertexMark, false, vertexCount * sizeof(bool));
            int* modelVertices = new int[vertexCount];
            
            stream->seekg(modelsAddr, std::ios::beg);
            for (int i = 0; i < modelCount; i++) {
                int32_t faceIndex, faceCount;
                int totalVertexCount = 0;
                int modelVertexCount = 0;
                
                stream->seekg(BspLayout::ModelFaceIndex, std::ios::cur);
                stream->read(reinterpret_cast<char *>(&faceIndex), sizeof(int32_t));
                stream->read(reinterpret_cast<char *>(&faceCount), sizeof(int32_t));
                
                BspFaceList bspFaces;
                for (int j = 0; j < faceCount; j++) {
                    BspFaceInfo& faceInfo = faces[faceIndex + j];
                    BspTextureInfo* textureInfo = m_textureInfos[faceInfo.textureInfoIndex];
                    
                    int faceVertexCount = faceInfo.edgeCount;
                    std::vector<Vec3f> faceVertices;
                    for (int k = 0; k < faceInfo.edgeCount; k++) {
                        int faceEdgeIndex = faceEdges[faceInfo.edgeIndex + k];
                        int vertexIndex;
                        if (faceEdgeIndex < 0)
                            vertexIndex = edges[-faceEdgeIndex].vertex1;
                        else
                            vertexIndex = edges[faceEdgeIndex].vertex0;
                        faceVertices.push_back(vertices[vertexIndex]);
                        if (!vertexMark[vertexIndex]) {
                            vertexMark[vertexIndex] = true;
                            modelVertices[modelVertexCount++] = vertexIndex;
                        }
                    }
                    
                    BspFace* bspFace = new BspFace(textureInfo, faceVertices);
                    bspFaces.push_back(bspFace);
                    totalVertexCount += faceVertexCount;
                }
                
                Vec3f center;
                BBox bounds;
                
                center = vertices[modelVertices[0]];
                bounds.min = vertices[modelVertices[0]];
                bounds.max = vertices[modelVertices[0]];
                
                for (int i = 1; i < modelVertexCount; i++) {
                    int vertexIndex = modelVertices[i];
                    center += vertices[vertexIndex];
                    bounds.mergeWith(vertices[vertexIndex]);
                    vertexMark[vertexIndex] = false;
                }
                
                center /= static_cast<float>(modelVertexCount);
                
                BspModel* bspModel = new BspModel(bspFaces, totalVertexCount, center, bounds);
                m_models.push_back(bspModel);
            }
            
            delete [] faceEdges;
            delete [] vertexMark;
            delete [] modelVertices;
        }
        
        Bsp::~Bsp() {
            while(!m_textureInfos.empty()) delete m_textureInfos.back(), m_textureInfos.pop_back();
            while(!m_textures.empty()) delete m_textures.back(), m_textures.pop_back();
            while(!m_models.empty()) delete m_models.back(), m_models.pop_back();
        }
        
        BspManager* BspManager::sharedManager = NULL;
        
        const Bsp* BspManager::bsp(const String& name, const StringList& paths, Utility::Console& console) {
            String pathList = Utility::join(paths, ",");
            String key = pathList + ":" + name;
            
            BspMap::iterator it = m_bsps.find(key);
            if (it != m_bsps.end())
                return it->second;
            
            console.info("Loading '%s' (searching %s)", name.c_str(), pathList.c_str());
            
            IO::PakManager& pakManager = *IO::PakManager::sharedManager;
            IO::PakStream stream = pakManager.entryStream(name, paths);
            if (stream.get() != NULL) {
                Bsp* bsp = new Bsp(name, stream);
                m_bsps[key] = bsp;
                return bsp;
            }
            
            console.warn("Unable to find BSP '%s'", name.c_str());
            return NULL;
        }
        
        BspManager::BspManager() {}
        
        BspManager::~BspManager() {
            BspMap::iterator it, end;
            for (it = m_bsps.begin(), end = m_bsps.end(); it != end; ++it)
                delete it->second;
            m_bsps.clear();
        }
    }
}
