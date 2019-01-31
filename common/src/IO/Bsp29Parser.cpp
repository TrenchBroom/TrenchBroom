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
#include "IO/CharArrayReader.h"
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
            
            // static const size_t TextureNameLength     = 0x10;
            
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
        m_end(end),
        m_palette(palette) {}
        
        Assets::EntityModel* Bsp29Parser::doParseModel() {
            CharArrayReader reader(m_begin, m_end);
            const auto version = reader.readInt<int32_t>();
            if (version != 29) {
                throw AssetException() << "Unsupported BSP model version: " << version;
            }

            reader.seekFromBegin(BspLayout::DirTexturesAddress);
            const auto textureOffset = reader.readSize<int32_t>();
            /* const auto textureLength = */ reader.readSize<int32_t>();

            reader.seekFromBegin(BspLayout::DirTexInfosAddress);
            const auto textureInfoOffset = reader.readSize<int32_t>();
            const auto textureInfoLength = reader.readSize<int32_t>();
            const auto textureInfoCount  = textureInfoLength / BspLayout::TexInfoSize;

            reader.seekFromBegin(BspLayout::DirVerticesAddress);
            const auto vertexOffset = reader.readSize<int32_t>();
            const auto vertexLength = reader.readSize<int32_t>();
            const auto vertexCount = vertexLength / (3 * sizeof(float));

            reader.seekFromBegin(BspLayout::DirEdgesAddress);
            const auto edgeInfoOffset = reader.readSize<int32_t>();
            const auto edgeInfoLength = reader.readSize<int32_t>();
            const auto edgeInfoCount = edgeInfoLength / (2 * sizeof(uint16_t));

            reader.seekFromBegin(BspLayout::DirFacesAddress);
            const auto faceInfoOffset = reader.readSize<int32_t>();
            const auto faceInfoLength = reader.readSize<int32_t>();
            const auto faceInfoCount = faceInfoLength / BspLayout::FaceSize;

            reader.seekFromBegin(BspLayout::DirFaceEdgesAddress);
            const auto faceEdgesOffset = reader.readSize<int32_t>();
            const auto faceEdgesLength = reader.readSize<int32_t>();
            const auto faceEdgesCount = faceEdgesLength / BspLayout::FaceEdgeSize;

            reader.seekFromBegin(BspLayout::DirModelAddress);
            const auto modelsOffset = reader.readSize<int32_t>();
            const auto modelsLength = reader.readSize<int32_t>();
            const auto modelCount = modelsLength / BspLayout::ModelSize;

            const auto textures = parseTextures(reader.subReaderFromBegin(textureOffset));
            const auto textureInfos = parseTextureInfos(reader.subReaderFromBegin(textureInfoOffset), textureInfoCount);
            const auto vertices = parseVertices(reader.subReaderFromBegin(vertexOffset), vertexCount);
            const auto edgeInfos = parseEdgeInfos(reader.subReaderFromBegin(edgeInfoOffset), edgeInfoCount);
            const auto faceInfos = parseFaceInfos(reader.subReaderFromBegin(faceInfoOffset), faceInfoCount);
            const auto faceEdges = parseFaceEdges(reader.subReaderFromBegin(faceEdgesOffset), faceEdgesCount);

            return parseModels(reader.subReaderFromBegin(modelsOffset), modelCount, textures, textureInfos, vertices, edgeInfos, faceInfos, faceEdges);
        }

        Assets::TextureList Bsp29Parser::parseTextures(CharArrayReader reader) {
            const TextureReader::TextureNameStrategy nameStrategy;
            IdMipTextureReader textureReader(nameStrategy, m_palette);

            const auto textureCount = reader.readSize<int32_t>();
            Assets::TextureList result(textureCount);

            for (size_t i = 0; i < textureCount; ++i) {
                const auto textureOffset = reader.readInt<int32_t>();
                // 2153: Some BSPs contain negative texture offsets.
                if (textureOffset < 0) {
                    continue;
                }

                auto subReader = reader.subReaderFromBegin(static_cast<size_t>(textureOffset));

                // We can't easily tell where the texture ends without duplicating all of the parsing code (including HlMip) here.
                // Just prevent the texture reader from reading past the end of the .bsp file.
                auto fileView = std::make_shared<MappedFileBufferView>(Path(), subReader.begin(), subReader.end());
                result[i] = textureReader.readTexture(fileView);
            }

            return result;
        }
        
        Bsp29Parser::TextureInfoList Bsp29Parser::parseTextureInfos(CharArrayReader reader, const size_t textureInfoCount) {
            TextureInfoList result(textureInfoCount);
            for (size_t i = 0; i < textureInfoCount; ++i) {
                result[i].sAxis = reader.readVec<float, 3>();
                result[i].sOffset = reader.readFloat<float>();
                result[i].tAxis = reader.readVec<float, 3>();
                result[i].tOffset = reader.readFloat<float>();
                result[i].textureIndex = reader.readSize<uint32_t>();
                reader.seekForward(BspLayout::TexInfoRest);
            }
            return result;
        }
        
        std::vector<vm::vec3f> Bsp29Parser::parseVertices(CharArrayReader reader, const size_t vertexCount) {
            std::vector<vm::vec3f> result(vertexCount);
            for (size_t i = 0; i < vertexCount; ++i) {
                result[i] = reader.readVec<float, 3>();
            }
            return result;
        }
        
        Bsp29Parser::EdgeInfoList Bsp29Parser::parseEdgeInfos(CharArrayReader reader, const size_t edgeInfoCount) {
            EdgeInfoList result(edgeInfoCount);
            for (size_t i = 0; i < edgeInfoCount; ++i) {
                result[i].vertexIndex1 = reader.readSize<uint16_t>();
                result[i].vertexIndex2 = reader.readSize<uint16_t>();
            }
            return result;
        }
        
        Bsp29Parser::FaceInfoList Bsp29Parser::parseFaceInfos(CharArrayReader reader, const size_t faceInfoCount) {
            FaceInfoList result(faceInfoCount);
            for (size_t i = 0; i < faceInfoCount; ++i) {
                reader.seekForward(BspLayout::FaceEdgeIndex);
                result[i].edgeIndex = reader.readSize<int32_t>();
                result[i].edgeCount = reader.readSize<uint16_t>();
                result[i].textureInfoIndex = reader.readSize<uint16_t>();
                reader.seekForward(BspLayout::FaceRest);
            }
            return result;
        }
        
        Bsp29Parser::FaceEdgeIndexList Bsp29Parser::parseFaceEdges(CharArrayReader reader, const size_t faceEdgeCount) {
            FaceEdgeIndexList result(faceEdgeCount);
            for (size_t i = 0; i < faceEdgeCount; ++i) {
                result[i] = reader.readInt<int32_t>();
            }
            return result;
        }

        Assets::EntityModel* Bsp29Parser::parseModels(CharArrayReader reader, const size_t modelCount, const Assets::TextureList& skins, const TextureInfoList& textureInfos, const std::vector<vm::vec3f>& vertices, const EdgeInfoList& edgeInfos, const FaceInfoList& faceInfos, const FaceEdgeIndexList& faceEdges) {
            using Vertex = Assets::EntityModel::Vertex;
            using VertexList = Vertex::List;

            auto model = std::make_unique<Assets::EntityModel>(m_name);
            auto& surface = model->addSurface(m_name);

            for (auto* skin : skins) {
                surface.addSkin(skin);
            }

            for (size_t i = 0; i < modelCount; ++i) {
                reader.seekForward(BspLayout::ModelFaceIndex);
                const auto modelFaceIndex = reader.readSize<int32_t>();
                const auto modelFaceCount = reader.readSize<int32_t>();
                size_t totalVertexCount = 0;
                Renderer::TexturedIndexRangeMap::Size size;

                for (size_t j = 0; j < modelFaceCount; ++j) {
                    const auto& faceInfo = faceInfos[modelFaceIndex + j];
                    const auto& textureInfo = textureInfos[faceInfo.textureInfoIndex];
                    auto* skin = skins[textureInfo.textureIndex];
                    if (skin != nullptr) {
                        const auto faceVertexCount = faceInfo.edgeCount;
                        size.inc(skin, GL_POLYGON, faceVertexCount);
                        totalVertexCount += faceVertexCount;
                    }
                }

                vm::bbox3f bounds;

                Renderer::TexturedIndexRangeMapBuilder<Vertex::Spec> builder(totalVertexCount, size);
                for (size_t j = 0; j < modelFaceCount; ++j) {
                    const auto& faceInfo = faceInfos[modelFaceIndex + j];
                    const auto& textureInfo = textureInfos[faceInfo.textureInfoIndex];
                    auto* skin = skins[textureInfo.textureIndex];
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

                            if (i == 0 && j == 0) {
                                bounds.min = bounds.max = position;
                            } else {
                                bounds = vm::merge(bounds, position);
                            }

                            faceVertices.push_back(Vertex(position, texCoords));
                        }

                        builder.addPolygon(skin, faceVertices);
                    }
                }

                StringStream frameName;
                frameName << m_name << "_" << i;
                model->addFrame(frameName.str(), bounds);

                surface.addTexturedMesh(builder.vertices(), builder.indices());
            }

            return model.release();
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
