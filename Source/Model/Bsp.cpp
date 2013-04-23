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

#include "IO/IOUtils.h"
#include "Utility/List.h"

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

        BspModel::BspModel(const BspFaceList& faces, unsigned int vertexCount, const Vec3f& center, const BBoxf& bounds) :
        m_faces(faces),
        m_vertexCount(vertexCount),
        m_center(center),
        m_bounds(bounds) {}

        BspModel::~BspModel() {
            Utility::deleteAll(m_faces);
        }

        void Bsp::readTextures(char*& cursor, unsigned int count) {
            using namespace IO;
            
            char textureName[BspLayout::TextureNameLength + 1];
            textureName[BspLayout::TextureNameLength] = 0;
            
            char* base = cursor;
            for (unsigned int i = 0; i < count; i++) {
                cursor = base + (i + 1) * sizeof(int32_t);
                int textureOffset = readInt<int32_t>(cursor);
                
                cursor = base + textureOffset;
                readBytes(cursor, textureName, BspLayout::TextureNameLength);
                unsigned int width = readUnsignedInt<uint32_t>(cursor);
                unsigned int height = readUnsignedInt<uint32_t>(cursor);
                unsigned int mip0Offset = readUnsignedInt<uint32_t>(cursor);

                unsigned char* mip0 = new unsigned char[width * height];
                cursor = base + textureOffset + mip0Offset;
                readBytes(cursor, mip0, width * height);

                BspTexture* texture = new BspTexture(textureName, mip0, width, height);
                m_textures[i] = texture;
            }
        }

        void Bsp::readTextureInfos(char*& cursor, unsigned int count, std::vector<BspTexture*>& textures) {
            using namespace IO;
            
            for (unsigned int i = 0; i < count; i++) {
                BspTextureInfo* textureInfo = new BspTextureInfo();
                textureInfo->sAxis = readVec3f(cursor);
                textureInfo->sOffset = readFloat<float>(cursor);
                textureInfo->tAxis = readVec3f(cursor);
                textureInfo->tOffset = readFloat<float>(cursor);

                unsigned int textureIndex = readUnsignedInt<uint32_t>(cursor);
                textureInfo->texture = textures[textureIndex];

                m_textureInfos[i] = textureInfo;
                cursor += BspLayout::TexInfoRest;
            }
        }

        void Bsp::readVertices(char*& cursor, unsigned int count, Vec3f::List& vertices) {
            using namespace IO;
            
            vertices.reserve(count);
            for (unsigned int i = 0; i < count; i++) {
                Vec3f vertex = readVec3f(cursor);
                vertices.push_back(vertex);
            }
        }

        void Bsp::readEdges(char*& cursor, unsigned int count, BspEdgeInfoList& edges) {
            using namespace IO;
            
            edges.reserve(count);
            BspEdgeInfo edgeInfo;
            for (unsigned int i = 0; i < count; i++) {
                edgeInfo.vertex0 = readUnsignedInt<uint16_t>(cursor);
                edgeInfo.vertex1 = readUnsignedInt<uint16_t>(cursor);
                edges.push_back(edgeInfo);
            }
        }

        void Bsp::readFaces(char*& cursor, unsigned int count, BspFaceInfoList& faces) {
            using namespace IO;
            
            faces.reserve(count);
            BspFaceInfo face;
            for (unsigned int i = 0; i < count; i++) {
                cursor += BspLayout::FaceEdgeIndex;

                face.edgeIndex = readUnsignedInt<int32_t>(cursor);
                face.edgeCount = readUnsignedInt<uint16_t>(cursor);
                face.textureInfoIndex = readUnsignedInt<uint16_t>(cursor);
                faces.push_back(face);

                cursor += BspLayout::FaceRest;
            }
        }

        void Bsp::readFaceEdges(char*& cursor, unsigned int count, BspFaceEdgeIndexList& indices) {
            using namespace IO;
            
            indices.reserve(count);
            for (unsigned int i = 0; i < count; i++) {
                int index = readInt<int32_t>(cursor);
                indices.push_back(index);
            }
        }

        Bsp::Bsp(const String& name, char* begin, char* end) :
        m_name(name) {
            using namespace IO;
            
            char* cursor = begin;
            int version = readInt<int32_t>(cursor); version = version; // prevent warning
            cursor = begin + BspLayout::DirTexturesAddress;
            int textureAddr = readInt<int32_t>(cursor);
            cursor = begin + textureAddr;
            unsigned int textureCount = readUnsignedInt<int32_t>(cursor);
            
            cursor -= sizeof(int32_t);
            m_textures.resize(size_t(textureCount));
            readTextures(cursor, textureCount);

            cursor = begin + BspLayout::DirTexInfosAddress;
            int texInfosAddr = readInt<int32_t>(cursor);
            unsigned int texInfosLength = readUnsignedInt<int32_t>(cursor);
            unsigned int texInfoCount = texInfosLength / BspLayout::TexInfoSize;
            m_textureInfos.resize(texInfoCount);
            cursor = begin + texInfosAddr;
            readTextureInfos(cursor, texInfoCount, m_textures);

            cursor = begin + BspLayout::DirVerticesAddress;
            int verticesAddr = readInt<int32_t>(cursor);
            unsigned int verticesLength = readUnsignedInt<int32_t>(cursor);
            unsigned int vertexCount = verticesLength / sizeof(Vec3f);
            Vec3f::List vertices;
            cursor = begin + verticesAddr;
            readVertices(cursor, vertexCount, vertices);

            cursor = begin + BspLayout::DirEdgesAddress;
            int edgesAddr = readInt<int32_t>(cursor);
            unsigned int edgesLength = readUnsignedInt<int32_t>(cursor);
            unsigned int edgeCount = edgesLength / (2 * sizeof(uint16_t));
            BspEdgeInfoList edges;
            cursor = begin + edgesAddr;
            readEdges(cursor, edgeCount, edges);

            cursor = begin + BspLayout::DirFacesAddress;
            int facesAddr = readInt<int32_t>(cursor);
            unsigned int facesLength = readUnsignedInt<int32_t>(cursor);
            unsigned int faceCount = facesLength / BspLayout::FaceSize;
            BspFaceInfoList faces;
            cursor = begin + facesAddr;
            readFaces(cursor, faceCount, faces);

            cursor = begin + BspLayout::DirFaceEdgesAddress;
            int faceEdgesAddr = readInt<int32_t>(cursor);
            unsigned int faceEdgesLength = readUnsignedInt<int32_t>(cursor);
            unsigned int faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;
            BspFaceEdgeIndexList faceEdges;
            cursor = begin + faceEdgesAddr;
            readFaceEdges(cursor, faceEdgesCount, faceEdges);

            cursor = begin + BspLayout::DirModelAddress;
            int modelsAddr = readInt<int32_t>(cursor);
            unsigned int modelsLength = readUnsignedInt<int32_t>(cursor);
            unsigned int modelCount = modelsLength / BspLayout::ModelSize;

            typedef std::vector<bool> MarkList;
            MarkList vertexMarks;
            vertexMarks.resize(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) vertexMarks[i] = false;
            
            typedef std::vector<unsigned int> ModelVertexIndexList;
            ModelVertexIndexList modelVertices;
            modelVertices.resize(vertexCount);

            cursor = begin + modelsAddr;
            for (unsigned int i = 0; i < modelCount; i++) {
                cursor += BspLayout::ModelFaceIndex;
                unsigned int modelFaceIndex = readUnsignedInt<int32_t>(cursor);
                unsigned int modelFaceCount = readUnsignedInt<int32_t>(cursor);
                
                unsigned int totalVertexCount = 0;
                unsigned int modelVertexCount = 0;

                BspFaceList bspFaces;
                for (unsigned int j = 0; j < static_cast<unsigned int>(modelFaceCount); j++) {
                    BspFaceInfo& faceInfo = faces[modelFaceIndex + j];
                    BspTextureInfo* textureInfo = m_textureInfos[faceInfo.textureInfoIndex];

                    unsigned int faceVertexCount = faceInfo.edgeCount;
                    std::vector<Vec3f> faceVertices;
                    for (unsigned int k = 0; k < faceInfo.edgeCount; k++) {
                        int faceEdgeIndex = faceEdges[faceInfo.edgeIndex + k];
                        unsigned int vertexIndex;
                        if (faceEdgeIndex < 0)
                            vertexIndex = edges[static_cast<size_t>(-faceEdgeIndex)].vertex1;
                        else
                            vertexIndex = edges[static_cast<size_t>(faceEdgeIndex)].vertex0;
                        
                        faceVertices.push_back(vertices[vertexIndex]);
                        if (!vertexMarks[vertexIndex]) {
                            vertexMarks[vertexIndex] = true;
                            modelVertices[modelVertexCount++] = vertexIndex;
                        }
                    }

                    BspFace* bspFace = new BspFace(textureInfo, faceVertices);
                    bspFaces.push_back(bspFace);
                    totalVertexCount += static_cast<unsigned int>(faceVertexCount);
                }

                Vec3f center;
                BBoxf bounds;

                center = vertices[modelVertices[0]];
                bounds.min = vertices[modelVertices[0]];
                bounds.max = vertices[modelVertices[0]];

                for (unsigned int j = 1; j < modelVertexCount; j++) {
                    unsigned int vertexIndex = modelVertices[j];
                    center += vertices[vertexIndex];
                    bounds.mergeWith(vertices[vertexIndex]);
                    vertexMarks[vertexIndex] = false;
                }

                center /= static_cast<float>(modelVertexCount);

                BspModel* bspModel = new BspModel(bspFaces, totalVertexCount, center, bounds);
                m_models.push_back(bspModel);
            }
        }

        Bsp::~Bsp() {
            Utility::deleteAll(m_textureInfos);
            Utility::deleteAll(m_textures);
            Utility::deleteAll(m_models);
        }

        BspManager* BspManager::sharedManager = NULL;

        const Bsp* BspManager::bsp(const String& name, const StringList& paths, Utility::Console& console) {
            String pathList = Utility::join(paths, ",");
            String key = pathList + ":" + name;

            BspMap::iterator it = m_bsps.find(key);
            if (it != m_bsps.end())
                return it->second;

            console.info("Loading '%s' (searching %s)", name.c_str(), pathList.c_str());

            IO::MappedFile::Ptr file = IO::findGameFile(name, paths);
            if (file.get() != NULL) {
                Bsp* bsp = new Bsp(name, file->begin(), file->end());
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
