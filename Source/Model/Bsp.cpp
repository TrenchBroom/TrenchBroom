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

        BspModel::BspModel(const BspFaceList& faces, unsigned int vertexCount, const Vec3f& center, const BBox& bounds) :
        m_faces(faces),
        m_vertexCount(vertexCount),
        m_center(center),
        m_bounds(bounds) {}

        BspModel::~BspModel() {
            Utility::deleteAll(m_faces);
        }

        void Bsp::readTextures(IO::PakStream& stream, unsigned int count) {
            char textureName[BspLayout::TextureNameLength + 1];
            textureName[BspLayout::TextureNameLength] = 0;
            
            const std::streampos base = stream->tellg();
            for (unsigned int i = 0; i < count; i++) {
                std::streamoff streamOffset = (i + 1) * sizeof(int32_t);
                streamOffset += base;
                stream->seekg(streamOffset, std::ios::beg);

                std::streamoff textureOffset = IO::readInt<int32_t>(stream);
                textureOffset += base;
                stream->seekg(textureOffset, std::ios::beg);
                
                stream->read(textureName, BspLayout::TextureNameLength);
                unsigned int width = IO::readUnsignedInt<uint32_t>(stream);
                unsigned int height = IO::readUnsignedInt<uint32_t>(stream);
                unsigned char* mip0 = new unsigned char[width * height];

                std::streamoff mip0Offset = IO::readUnsignedInt<uint32_t>(stream);
                mip0Offset += textureOffset;
                stream->seekg(mip0Offset, std::ios::beg);
                stream->read(reinterpret_cast<char *>(mip0), static_cast<std::streamsize>(width * height));

                BspTexture* texture = new BspTexture(textureName, mip0, width, height);
                m_textures[i] = texture;
            }
        }

        void Bsp::readTextureInfos(IO::PakStream& stream, unsigned int count, std::vector<BspTexture*>& textures) {

            for (unsigned int i = 0; i < count; i++) {
                BspTextureInfo* textureInfo = new BspTextureInfo();
                textureInfo->sAxis = IO::readVec3f(stream);
                textureInfo->sOffset = IO::readFloat(stream);
                textureInfo->tAxis = IO::readVec3f(stream);
                textureInfo->tOffset = IO::readFloat(stream);

                unsigned int textureIndex = IO::readUnsignedInt<uint32_t>(stream);
                textureInfo->texture = textures[textureIndex];
                stream->seekg(BspLayout::TexInfoRest, std::ios::cur);

                m_textureInfos[i] = textureInfo;
            }
        }

        void Bsp::readVertices(IO::PakStream& stream, unsigned int count, Vec3f::List& vertices) {
            vertices.reserve(count);
            for (unsigned int i = 0; i < count; i++) {
                Vec3f vertex = IO::readVec3f(stream);
                vertices.push_back(vertex);
            }
        }

        void Bsp::readEdges(IO::PakStream& stream, unsigned int count, BspEdgeInfoList& edges) {
            edges.reserve(count);
            BspEdgeInfo edgeInfo;
            for (unsigned int i = 0; i < count; i++) {
                edgeInfo.vertex0 = IO::readUnsignedInt<uint16_t>(stream);
                edgeInfo.vertex1 = IO::readUnsignedInt<uint16_t>(stream);
                edges.push_back(edgeInfo);
            }
        }

        void Bsp::readFaces(IO::PakStream& stream, unsigned int count, BspFaceInfoList& faces) {
            faces.reserve(count);
            BspFaceInfo face;
            for (unsigned int i = 0; i < count; i++) {
                stream->seekg(BspLayout::FaceEdgeIndex, std::ios::cur);

                face.edgeIndex = IO::readUnsignedInt<int32_t>(stream);
                face.edgeCount = IO::readUnsignedInt<uint16_t>(stream);
                face.textureInfoIndex = IO::readUnsignedInt<uint16_t>(stream);
                faces.push_back(face);

                stream->seekg(BspLayout::FaceRest, std::ios::cur);
            }
        }

        void Bsp::readFaceEdges(IO::PakStream& stream, unsigned int count, BspFaceEdgeIndexList& indices) {
            indices.reserve(count);
            for (unsigned int i = 0; i < count; i++) {
                int index = IO::readInt<int32_t>(stream);
                indices.push_back(index);
            }
        }

        Bsp::Bsp(const String& name, IO::PakStream stream) :
        m_name(name) {
            int version = IO::readInt<int32_t>(stream); version = version; // prevent warning
            stream->seekg(BspLayout::DirTexturesAddress, std::ios::beg);
            int textureAddr = IO::readInt<int32_t>(stream);
            stream->seekg(textureAddr, std::ios::beg);
            unsigned int textureCount = IO::readUnsignedInt<int32_t>(stream);
            
            stream->seekg(-static_cast<std::streamoff>(sizeof(int32_t)), std::ios::cur);
            m_textures.resize(size_t(textureCount));
            readTextures(stream, textureCount);

            stream->seekg(BspLayout::DirTexInfosAddress, std::ios::beg);
            int texInfosAddr = IO::readInt<int32_t>(stream);
            unsigned int texInfosLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int texInfoCount = texInfosLength / BspLayout::TexInfoSize;
            m_textureInfos.resize(texInfoCount);
            stream->seekg(texInfosAddr, std::ios::beg);
            readTextureInfos(stream, texInfoCount, m_textures);

            stream->seekg(BspLayout::DirVerticesAddress, std::ios::beg);
            int verticesAddr = IO::readInt<int32_t>(stream);
            unsigned int verticesLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int vertexCount = verticesLength / sizeof(Vec3f);
            Vec3f::List vertices;
            stream->seekg(verticesAddr, std::ios::beg);
            readVertices(stream, vertexCount, vertices);

            stream->seekg(BspLayout::DirEdgesAddress, std::ios::beg);
            int edgesAddr = IO::readInt<int32_t>(stream);
            unsigned int edgesLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int edgeCount = edgesLength / (2 * sizeof(uint16_t));
            BspEdgeInfoList edges;
            stream->seekg(edgesAddr, std::ios::beg);
            readEdges(stream, edgeCount, edges);

            stream->seekg(BspLayout::DirFacesAddress, std::ios::beg);
            int facesAddr = IO::readInt<int32_t>(stream);
            unsigned int facesLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int faceCount = facesLength / BspLayout::FaceSize;
            BspFaceInfoList faces;
            stream->seekg(facesAddr, std::ios::beg);
            readFaces(stream, faceCount, faces);

            stream->seekg(BspLayout::DirFaceEdgesAddress, std::ios::beg);
            int faceEdgesAddr = IO::readInt<int32_t>(stream);
            unsigned int faceEdgesLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;
            BspFaceEdgeIndexList faceEdges;
            stream->seekg(faceEdgesAddr, std::ios::beg);
            readFaceEdges(stream, faceEdgesCount, faceEdges);

            stream->seekg(BspLayout::DirModelAddress, std::ios::beg);
            int modelsAddr = IO::readInt<int32_t>(stream);
            unsigned int modelsLength = IO::readUnsignedInt<int32_t>(stream);
            unsigned int modelCount = modelsLength / BspLayout::ModelSize;

            typedef std::vector<bool> MarkList;
            MarkList vertexMarks;
            vertexMarks.resize(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) vertexMarks[i] = false;
            
            typedef std::vector<unsigned int> ModelVertexIndexList;
            ModelVertexIndexList modelVertices;
            modelVertices.resize(vertexCount);

            stream->seekg(modelsAddr, std::ios::beg);
            for (unsigned int i = 0; i < modelCount; i++) {
                stream->seekg(BspLayout::ModelFaceIndex, std::ios::cur);
                unsigned int modelFaceIndex = IO::readUnsignedInt<int32_t>(stream);
                unsigned int modelFaceCount = IO::readUnsignedInt<int32_t>(stream);
                
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
                BBox bounds;

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
