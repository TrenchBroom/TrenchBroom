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
#include <numeric>
#include <cstring>
#include "Utilities/Console.h"

#define BSP_DIR_TEXTURES_ADDRESS      0x14
#define BSP_DIR_TEXTURES_SIZE         0x18
#define BSP_DIR_VERTICES_ADDRESS      0x1C
#define BSP_DIR_VERTICES_SIZE         0x20
#define BSP_DIR_TEXINFOS_ADDRESS      0x34
#define BSP_DIR_TEXINFOS_SIZE         0x38
#define BSP_DIR_FACES_ADDRESS         0x3C
#define BSP_DIR_FACES_SIZE            0x40
#define BSP_DIR_EDGES_ADDRESS         0x64
#define BSP_DIR_EDGES_SIZE            0x68
#define BSP_DIR_FACE_EDGES_ADDRESS    0x6C
#define BSP_DIR_FACE_EDGES_SIZE       0x70
#define BSP_DIR_MODEL_ADDRESS         0x74
#define BSP_DIR_MODEL_SIZE            0x78

#define BSP_TEXTURE_NAME_LENGTH       0x10

#define BSP_FACE_SIZE                 0x14
#define BSP_FACE_EDGE_INDEX           0x4
#define BSP_FACE_REST                 0x8

#define BSP_TEXINFO_SIZE              0x28
#define BSP_TEXINFO_REST              0x4

#define BSP_FACE_EDGE_SIZE            0x4

#define BSP_MODEL_SIZE                0x40
#define BSP_MODEL_ORIGIN              0x18
#define BSP_MODEL_FACE_INDEX          0x38
#define BSP_MODEL_FACE_COUNT          0x3C

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            BspTexture::BspTexture(const std::string& name, const unsigned char* image, int width, int height)
            : name(name), image(image), width(width), height(height) {}

            BspTexture::~BspTexture() {
                delete[] image;
            }

            BspFace::BspFace(BspTextureInfo* textureInfo, const std::vector<Vec3f>& vertices)
            : textureInfo(textureInfo), vertices(vertices) {
                bounds.min = vertices.front();
                bounds.max = bounds.min;

                for (unsigned int i = 1; i < vertices.size(); i++)
                    bounds += vertices[i];
            }

            Vec2f BspFace::textureCoordinates(const Vec3f& vertex) {
                Vec2f result;
                result.x = ((vertex | textureInfo->sAxis) + textureInfo->sOffset) / textureInfo->texture->width;
                result.y = ((vertex | textureInfo->tAxis) + textureInfo->tOffset) / textureInfo->texture->height;
                return result;
            }

            BspModel::BspModel(const std::vector<BspFace*>& faces, int vertexCount, const Vec3f& center, const BBox& bounds)
            : faces(faces), vertexCount(vertexCount), center(center), bounds(bounds) {}

            BspModel::~BspModel() {
                while(!faces.empty()) delete faces.back(), faces.pop_back();
            }

            void Bsp::readTextures(IO::PakStream& stream, unsigned int count) {
                int32_t offset;
                uint32_t width, height, mip0Offset;
                char textureName[BSP_TEXTURE_NAME_LENGTH];
                unsigned char* mip0;
                std::streamoff base;

                base = stream->tellg();

                for (unsigned int i = 0; i < count; i++) {
                    stream->seekg(base + (i + 1) * sizeof(int32_t), std::ios::beg);
                    stream->read((char *)&offset, sizeof(int32_t));
                    stream->seekg(base + offset, std::ios::beg);
                    stream->read(textureName, BSP_TEXTURE_NAME_LENGTH);
                    stream->read((char *)&width, sizeof(uint32_t));
                    stream->read((char *)&height, sizeof(uint32_t));
                    stream->read((char *)&mip0Offset, sizeof(uint32_t));

                    mip0 = new unsigned char[width * height];
                    stream->seekg(base + offset + mip0Offset, std::ios::beg);
                    stream->read((char *)mip0, width * height);

                    BspTexture* texture = new BspTexture(textureName, mip0, width, height);
                    textures[i] = texture;
                }
            }

            void Bsp::readTextureInfos(IO::PakStream& stream, unsigned int count, std::vector<BspTexture*>& textures) {
                uint32_t textureIndex;

                for (unsigned int i = 0; i < count; i++) {
                    BspTextureInfo* textureInfo = new BspTextureInfo();
                    stream->read((char *)&textureInfo->sAxis, 3 * sizeof(float));
                    stream->read((char *)&textureInfo->sOffset, sizeof(float));
                    stream->read((char *)&textureInfo->tAxis, 3 * sizeof(float));
                    stream->read((char *)&textureInfo->tOffset, sizeof(float));
                    stream->read((char *)&textureIndex, sizeof(uint32_t));
                    textureInfo->texture = textures[textureIndex];
                    stream->seekg(BSP_TEXINFO_REST, std::ios::cur);

                    textureInfos[i] = textureInfo;
                }
            }

            void Bsp::readVertices(IO::PakStream& stream, unsigned int count, std::vector<Vec3f>& vertices) {
                Vec3f vertex;
                for (unsigned int i = 0; i < count; i++) {
                    stream->read((char *)&vertex, sizeof(Vec3f));
                    vertices.push_back(vertex);
                }
            }

            void Bsp::readEdges(IO::PakStream& stream, unsigned int count, std::vector<BspEdgeInfo>& edges) {
                BspEdgeInfo edge;
                for (unsigned int i = 0; i < count; i++) {
                    stream->read((char *)&edge, sizeof(BspEdgeInfo));
                    edges.push_back(edge);
                }
            }

            void Bsp::readFaces(IO::PakStream& stream, unsigned int count, std::vector<BspFaceInfo>& faces) {
                BspFaceInfo face;
                for (unsigned int i = 0; i < count; i++) {
                    stream->seekg(BSP_FACE_EDGE_INDEX, std::ios::cur);
                    stream->read((char *)&face.edgeIndex, sizeof(int32_t));
                    stream->read((char *)&face.edgeCount, sizeof(uint16_t));
                    stream->read((char *)&face.textureInfoIndex, sizeof(uint16_t));
                    stream->seekg(BSP_FACE_REST, std::ios::cur);

                    faces.push_back(face);
                }
            }

            void Bsp::readFaceEdges(IO::PakStream& stream, unsigned int count, int32_t* indices) {
                stream->read((char *)indices, count * sizeof(int32_t));
            }

            Bsp::Bsp(const std::string& name, IO::PakStream stream) : name(name) {
                int32_t version;
                stream->read((char *)&version, sizeof(int32_t));

                int32_t textureAddr, textureCount;
                stream->seekg(BSP_DIR_TEXTURES_ADDRESS, std::ios::beg);
                stream->read((char *)&textureAddr, sizeof(int32_t));

                stream->seekg(textureAddr, std::ios::beg);
                stream->read((char *)&textureCount, sizeof(int32_t));
                stream->seekg(-static_cast<int>(sizeof(int32_t)), std::ios::cur);
                textures.resize(textureCount);
                readTextures(stream, textureCount);

                int32_t texInfosAddr, texInfosLength;
                int texInfoCount;
                stream->seekg(BSP_DIR_TEXINFOS_ADDRESS, std::ios::beg);
                stream->read((char *)&texInfosAddr, sizeof(int32_t));
                stream->read((char *)&texInfosLength, sizeof(int32_t));
                texInfoCount = texInfosLength / BSP_TEXINFO_SIZE;
                textureInfos.resize(texInfoCount);
                stream->seekg(texInfosAddr, std::ios::beg);
                readTextureInfos(stream, texInfoCount, textures);

                int32_t verticesAddr, verticesLength;
                int vertexCount;
                stream->seekg(BSP_DIR_VERTICES_ADDRESS, std::ios::beg);
                stream->read((char *)&verticesAddr, sizeof(int32_t));
                stream->read((char *)&verticesLength, sizeof(int32_t));
                vertexCount = verticesLength / sizeof(Vec3f);

                std::vector<Vec3f> vertices;
                stream->seekg(verticesAddr, std::ios::beg);
                readVertices(stream, vertexCount, vertices);

                int32_t edgesAddr, edgesLength;
                int edgeCount;
                stream->seekg(BSP_DIR_EDGES_ADDRESS, std::ios::beg);
                stream->read((char *)&edgesAddr, sizeof(int32_t));
                stream->read((char *)&edgesLength, sizeof(int32_t));
                edgeCount = edgesLength / sizeof(BspEdgeInfo);

                std::vector<BspEdgeInfo> edges;
                stream->seekg(edgesAddr, std::ios::beg);
                readEdges(stream, edgeCount, edges);

                int32_t facesAddr, facesLength;
                int faceCount;
                stream->seekg(BSP_DIR_FACES_ADDRESS, std::ios::beg);
                stream->read((char *)&facesAddr, sizeof(int32_t));
                stream->read((char *)&facesLength, sizeof(int32_t));
                faceCount = facesLength / BSP_FACE_SIZE;

                std::vector<BspFaceInfo> faces;
                stream->seekg(facesAddr, std::ios::beg);
                readFaces(stream, faceCount, faces);

                int32_t faceEdgesAddr, faceEdgesLength;
                int faceEdgesCount;
                stream->seekg(BSP_DIR_FACE_EDGES_ADDRESS, std::ios::beg);
                stream->read((char *)&faceEdgesAddr, sizeof(int32_t));
                stream->read((char *)&faceEdgesLength, sizeof(int32_t));
                faceEdgesCount = faceEdgesLength / BSP_FACE_EDGE_SIZE;

                int32_t* faceEdges = new int32_t[faceEdgesCount];
                stream->seekg(faceEdgesAddr, std::ios::beg);
                readFaceEdges(stream, faceEdgesCount, faceEdges);

                int32_t modelsAddr, modelsLength;
                int modelCount;
                stream->seekg(BSP_DIR_MODEL_ADDRESS, std::ios::beg);
                stream->read((char *)&modelsAddr, sizeof(int32_t));
                stream->read((char *)&modelsLength, sizeof(int32_t));
                modelCount = modelsLength / BSP_MODEL_SIZE;

                bool* vertexMark = new bool[vertexCount];
                memset(vertexMark, false, vertexCount * sizeof(bool));
                int* modelVertices = new int[vertexCount];

                stream->seekg(modelsAddr, std::ios::beg);
                for (int i = 0; i < modelCount; i++) {
                    int32_t faceIndex, faceCount;
                    int totalVertexCount = 0;
                    int modelVertexCount = 0;

                    stream->seekg(BSP_MODEL_FACE_INDEX, std::ios::cur);
                    stream->read((char *)&faceIndex, sizeof(int32_t));
                    stream->read((char *)&faceCount, sizeof(int32_t));

                    std::vector<BspFace*> bspFaces;
                    for (int j = 0; j < faceCount; j++) {
                        BspFaceInfo& faceInfo = faces[faceIndex + j];
                        BspTextureInfo* textureInfo = textureInfos[faceInfo.textureInfoIndex];

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
                        bounds += vertices[vertexIndex];
                        vertexMark[vertexIndex] = false;
                    }

                    center /= static_cast<float>(modelVertexCount);

                    BspModel* bspModel = new BspModel(bspFaces, totalVertexCount, center, bounds);
                    models.push_back(bspModel);
                }

				delete [] faceEdges;
				delete [] vertexMark;
				delete [] modelVertices;
            }

            Bsp::~Bsp() {
                while(!textureInfos.empty()) delete textureInfos.back(), textureInfos.pop_back();
                while(!textures.empty()) delete textures.back(), textures.pop_back();
                while(!models.empty()) delete models.back(), models.pop_back();
            }

            BspManager* BspManager::sharedManager = NULL;
            
            Bsp* BspManager::bspForName(const std::string& name, const std::vector<std::string>& paths) {
                std::string pathList = accumulate(paths.begin(), paths.end(), std::string(";"));
                std::string key = pathList + ":" + name;

                std::map<std::string, Bsp*>::iterator it = bsps.find(key);
                if (it != bsps.end())
                    return it->second;

                log(TB_LL_INFO, "Loading BSP model '%s', search paths: %s\n", name.c_str(), pathList.c_str());

                IO::PakManager& pakManager = *IO::PakManager::sharedManager;
                IO::PakStream stream = pakManager.streamForEntry(name, paths);
                if (stream.get() != NULL) {
                    Bsp* bsp = new Bsp(name, stream);
                    bsps[key] = bsp;
                    return bsp;
                }

                log(TB_LL_WARN, "Unable to find BSP model '%s'\n", name.c_str());
                return NULL;
            }

            BspManager::BspManager() {}

            BspManager::~BspManager() {
                for (std::map<std::string, Bsp*>::iterator it = bsps.begin(); it != bsps.end(); ++it)
                    delete it->second;
            }
        }
    }
}
