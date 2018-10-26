/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Bsp29Parser.h"

#include <Assets/EntityModel.h>
#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/IOUtils.h"
#include "IO/IdMipTextureReader.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

namespace TrenchBroom {
    namespace IO {
        namespace BspLayout {
            static const size_t DirTexturesAddress    = 0x14;
            // static const size_t DirTexturesSize       = 0x18;
            static const size_t DirVerticesAddress    = 0x1C;
            // static const size_t DirVerticesSize       = 0x20;
            static const size_t DirTexInfosAddress    = 0x34;
            // static const size_t DirTexInfoSize        = 0x38;
            static const size_t DirFacesAddress       = 0x3C;
            // static const size_t DirFacesSize          = 0x40;
            static const size_t DirEdgesAddress       = 0x64;
            // static const size_t DirEdgesSize          = 0x68;
            static const size_t DirFaceEdgesAddress   = 0x6C;
            // static const size_t DirFaceEdgesSize      = 0x70;
            static const size_t DirModelAddress       = 0x74;
            // static const size_t DirModelSize          = 0x78;
            
            static const size_t TextureNameLength     = 0x10;
            
            static const size_t FaceSize              = 0x14;
            static const size_t FaceEdgeIndex         = 0x4;
            static const size_t FaceRest              = 0x8;
            
            static const size_t TexInfoSize           = 0x28;
            static const size_t TexInfoRest           = 0x4;
            
            static const size_t FaceEdgeSize          = 0x4;
            static const size_t ModelSize             = 0x40;
            // static const size_t ModelOrigin           = 0x18;
            static const size_t ModelFaceIndex        = 0x38;
            // static const size_t ModelFaceCount        = 0x3c;
        }
        
        Bsp29Parser::Bsp29Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette) :
        m_name(name),
        m_begin(begin),
        // m_end(end),
        m_palette(palette) {}
        
        Assets::EntityModel* Bsp29Parser::doParseModel() {
            const char* cursor = m_begin;
            const int version = readInt<int32_t>(cursor);
            assert(version == 29);
            unused(version);

            auto model = std::make_unique<Assets::EntityModel>(m_name);

            parseTextures(model.get());
            const TextureInfoList textureInfos = parseTextureInfos();
            const std::vector<vm::vec3f> vertices = parseVertices();
            const EdgeInfoList edgeInfos = parseEdgeInfos();
            const FaceInfoList faceInfos = parseFaceInfos();
            const FaceEdgeIndexList faceEdges = parseFaceEdges();
            
            parseModels(model.get(), textureInfos, vertices, edgeInfos, faceInfos, faceEdges);

            return model.release();
        }

        void Bsp29Parser::parseTextures(Assets::EntityModel* model) {
            char textureName[BspLayout::TextureNameLength + 1];
            textureName[BspLayout::TextureNameLength] = 0;
            Color averageColor;
            
            const char* cursor = m_begin + BspLayout::DirTexturesAddress;
            const size_t textureAddr = readSize<int32_t>(cursor);
            cursor = m_begin + textureAddr;
            const size_t textureCount = readSize<int32_t>(cursor);
            cursor -= sizeof(int32_t);

            const TextureReader::TextureNameStrategy nameStrategy;
            IdMipTextureReader textureReader(nameStrategy, m_palette);

            const char* base = cursor;
            for (size_t i = 0; i < textureCount; ++i) {
                cursor = base + (i + 1)*sizeof(int32_t);
                const int textureOffset = readInt<int32_t>(cursor);
                if (textureOffset < 0) {
                    continue;
                }
                cursor = base + textureOffset;
                readBytes(cursor, textureName, BspLayout::TextureNameLength);
                const size_t width = readSize<int32_t>(cursor);
                const size_t height = readSize<int32_t>(cursor);
                const size_t mipFileSize = IdMipTextureReader::mipFileSize(width, height, 4);
                
                const char* const begin = base + textureOffset;
                const char* const end = begin + mipFileSize;

                model->addSkin(textureReader.readTexture(begin, end, Path("")));
            }
        }
        
        Bsp29Parser::TextureInfoList Bsp29Parser::parseTextureInfos() {
            const char* cursor = m_begin + BspLayout::DirTexInfosAddress;
            const size_t texInfosAddr = readSize<int32_t>(cursor);
            const size_t texInfosLength = readSize<int32_t>(cursor);
            const size_t texInfoCount = texInfosLength/BspLayout::TexInfoSize;
            
            TextureInfoList textureInfos(texInfoCount);
            cursor = m_begin + texInfosAddr;
            for (size_t i = 0; i < texInfoCount; ++i) {
                textureInfos[i].sAxis = readVec3f(cursor);
                textureInfos[i].sOffset = readFloat<float>(cursor);
                textureInfos[i].tAxis = readVec3f(cursor);
                textureInfos[i].tOffset = readFloat<float>(cursor);
                textureInfos[i].textureIndex = readSize<uint32_t>(cursor);
                cursor += BspLayout::TexInfoRest;
            }
            return textureInfos;
        }
        
        std::vector<vm::vec3f> Bsp29Parser::parseVertices() {
            const char* cursor = m_begin + BspLayout::DirVerticesAddress;
            const size_t verticesAddr = readSize<int32_t>(cursor);
            const size_t verticesLength = readSize<int32_t>(cursor);
            const size_t vertexCount = verticesLength/(3*sizeof(float));
            
            std::vector<vm::vec3f> vertices(vertexCount);
            cursor = m_begin + verticesAddr;
            for (size_t i = 0; i < vertexCount; ++i)
                vertices[i] = readVec3f(cursor);
            return vertices;
        }
        
        Bsp29Parser::EdgeInfoList Bsp29Parser::parseEdgeInfos() {
            const char* cursor = m_begin + BspLayout::DirEdgesAddress;
            const size_t edgesAddr = readSize<int32_t>(cursor);
            const size_t edgesLength = readSize<int32_t>(cursor);
            const size_t edgeCount = edgesLength / (2*sizeof(uint16_t));
            
            EdgeInfoList edgeInfos(edgeCount);
            cursor = m_begin + edgesAddr;
            for (size_t i = 0; i < edgeCount; ++i) {
                edgeInfos[i].vertexIndex1 = readSize<uint16_t>(cursor);
                edgeInfos[i].vertexIndex2 = readSize<uint16_t>(cursor);
            }
            return edgeInfos;
        }
        
        Bsp29Parser::FaceInfoList Bsp29Parser::parseFaceInfos() {
            const char* cursor = m_begin + BspLayout::DirFacesAddress;
            const size_t facesAddr = readSize<int32_t>(cursor);
            const size_t facesLength = readSize<int32_t>(cursor);
            const size_t faceCount = facesLength / BspLayout::FaceSize;
            
            cursor = m_begin + facesAddr;
            FaceInfoList faceInfos(faceCount);
            for (size_t i = 0; i < faceCount; ++i) {
                cursor += BspLayout::FaceEdgeIndex;
                faceInfos[i].edgeIndex = readSize<int32_t>(cursor);
                faceInfos[i].edgeCount = readSize<uint16_t>(cursor);
                faceInfos[i].textureInfoIndex = readSize<uint16_t>(cursor);
                cursor += BspLayout::FaceRest;
            }
            return faceInfos;
        }
        
        Bsp29Parser::FaceEdgeIndexList Bsp29Parser::parseFaceEdges() {
            const char* cursor = m_begin + BspLayout::DirFaceEdgesAddress;
            const size_t faceEdgesAddr = readSize<int32_t>(cursor);
            const size_t faceEdgesLength = readSize<int32_t>(cursor);
            const size_t faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;
            
            cursor = m_begin + faceEdgesAddr;
            FaceEdgeIndexList faceEdges(faceEdgesCount);
            for (size_t i = 0; i < faceEdgesCount; ++i)
                faceEdges[i] = readInt<int32_t>(cursor);
            return faceEdges;
        }
        
        void Bsp29Parser::parseModels(Assets::EntityModel* model, const TextureInfoList& textureInfos, const std::vector<vm::vec3f>& vertices, const EdgeInfoList& edgeInfos, const FaceInfoList& faceInfos, const FaceEdgeIndexList& faceEdges) {
            using Vertex = Assets::EntityModel::Vertex;
            using VertexList = Vertex::List;

            const char* cursor = m_begin + BspLayout::DirModelAddress;
            const auto modelsAddr = readSize<int32_t>(cursor);
            const auto modelsLength = readSize<int32_t>(cursor);
            const auto modelCount = modelsLength / BspLayout::ModelSize;

            cursor = m_begin + modelsAddr;
            for (size_t i = 0; i < modelCount; ++i) {
                cursor += BspLayout::ModelFaceIndex;
                const auto modelFaceIndex = readSize<int32_t>(cursor);
                const auto modelFaceCount = readSize<int32_t>(cursor);
                size_t totalVertexCount = 0;
                Renderer::TexturedIndexRangeMap::Size size;

                for (size_t j = 0; j < modelFaceCount; ++j) {
                    const auto& faceInfo = faceInfos[modelFaceIndex + j];
                    const auto& textureInfo = textureInfos[faceInfo.textureInfoIndex];
                    auto* skin = model->skin(textureInfo.textureIndex);
                    if (skin != nullptr) {
                        const auto faceVertexCount = faceInfo.edgeCount;
                        size.inc(skin, GL_POLYGON, faceVertexCount);
                        totalVertexCount += faceVertexCount;
                    }
                }

                Renderer::TexturedIndexRangeMapBuilder<Vertex::Spec> builder(totalVertexCount, size);
                for (size_t j = 0; j < modelFaceCount; ++j) {
                    const auto& faceInfo = faceInfos[modelFaceIndex + j];
                    const auto& textureInfo = textureInfos[faceInfo.textureInfoIndex];
                    auto* skin = model->skin(textureInfo.textureIndex);
                    if (skin != nullptr) {
                        const auto faceVertexCount = faceInfo.edgeCount;

                        VertexList faceVertices;
                        faceVertices.reserve(faceVertexCount);
                        for (size_t k = 0; k < faceVertexCount; ++k) {
                            const int faceEdgeIndex = faceEdges[faceInfo.edgeIndex + k];
                            size_t vertexIndex;
                            if (faceEdgeIndex < 0) {
                                vertexIndex = edgeInfos[static_cast<size_t>(-faceEdgeIndex)].vertexIndex2;
                            } else {
                                vertexIndex = edgeInfos[static_cast<size_t>(faceEdgeIndex)].vertexIndex1;
                            }

                            const auto& position = vertices[vertexIndex];
                            const auto texCoords = textureCoords(position, textureInfo, skin);
                            faceVertices.push_back(Vertex(position, texCoords));
                        }

                        builder.addPolygon(skin, faceVertices);
                    }
                }

                StringStream frameName;
                frameName << m_name << "_" << i;
                model->addFrame(frameName.str(), builder.vertices(), builder.indices());
            }
        }
        
        vm::vec2f Bsp29Parser::textureCoords(const vm::vec3f& vertex, const TextureInfo& textureInfo, const Assets::Texture* texture) const {
            if (texture == nullptr) {
                return vm::vec2f::zero;
            } else {
                return vm::vec2f((dot(vertex, textureInfo.sAxis) + textureInfo.sOffset) / texture->width(),
                                 (dot(vertex, textureInfo.tAxis) + textureInfo.tOffset) / texture->height());
            }
        }
    }
}
