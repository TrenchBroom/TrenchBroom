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

#include "MdlParser.h"

#include "Exceptions.h"
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "Assets/Palette.h"
#include "IO/Reader.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include <kdl/string_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        namespace MdlLayout {
            static const int Ident = (('O'<<24) + ('P'<<16) + ('D'<<8) + 'I');
            static const int Version6 = 6;

            static const unsigned int HeaderNumSkins    = 0x30;
            static const unsigned int Skins             = 0x54;
            static const unsigned int SimpleFrameName   = 0x8;
            static const unsigned int SimpleFrameLength = 0x10;
            static const unsigned int MultiFrameTimes   = 0xC;
            // static const unsigned int FrameVertexSize   = 0x4;
        }

        const vm::vec3f MdlParser::Normals[] = {
            vm::vec3f(-0.525731f,  0.000000f,  0.850651f),
            vm::vec3f(-0.442863f,  0.238856f,  0.864188f),
            vm::vec3f(-0.295242f,  0.000000f,  0.955423f),
            vm::vec3f(-0.309017f,  0.500000f,  0.809017f),
            vm::vec3f(-0.162460f,  0.262866f,  0.951056f),
            vm::vec3f(-0.000000f,  0.000000f,  1.000000f),
            vm::vec3f(-0.000000f,  0.850651f,  0.525731f),
            vm::vec3f(-0.147621f,  0.716567f,  0.681718f),
            vm::vec3f(-0.147621f,  0.716567f,  0.681718f),
            vm::vec3f(-0.000000f,  0.525731f,  0.850651f),
            vm::vec3f(-0.309017f,  0.500000f,  0.809017f),
            vm::vec3f(-0.525731f,  0.000000f,  0.850651f),
            vm::vec3f(-0.295242f,  0.000000f,  0.955423f),
            vm::vec3f(-0.442863f,  0.238856f,  0.864188f),
            vm::vec3f(-0.162460f,  0.262866f,  0.951056f),
            vm::vec3f(-0.681718f,  0.147621f,  0.716567f),
            vm::vec3f(-0.809017f,  0.309017f,  0.500000f),
            vm::vec3f(-0.587785f,  0.425325f,  0.688191f),
            vm::vec3f(-0.850651f,  0.525731f,  0.000000f),
            vm::vec3f(-0.864188f,  0.442863f,  0.238856f),
            vm::vec3f(-0.716567f,  0.681718f,  0.147621f),
            vm::vec3f(-0.688191f,  0.587785f,  0.425325f),
            vm::vec3f(-0.500000f,  0.809017f,  0.309017f),
            vm::vec3f(-0.238856f,  0.864188f,  0.442863f),
            vm::vec3f(-0.425325f,  0.688191f,  0.587785f),
            vm::vec3f(-0.716567f,  0.681718f, -0.147621f),
            vm::vec3f(-0.500000f,  0.809017f, -0.309017f),
            vm::vec3f(-0.525731f,  0.850651f,  0.000000f),
            vm::vec3f(-0.000000f,  0.850651f, -0.525731f),
            vm::vec3f(-0.238856f,  0.864188f, -0.442863f),
            vm::vec3f(-0.000000f,  0.955423f, -0.295242f),
            vm::vec3f(-0.262866f,  0.951056f, -0.162460f),
            vm::vec3f(-0.000000f,  1.000000f,  0.000000f),
            vm::vec3f(-0.000000f,  0.955423f,  0.295242f),
            vm::vec3f(-0.262866f,  0.951056f,  0.162460f),
            vm::vec3f(-0.238856f,  0.864188f,  0.442863f),
            vm::vec3f(-0.262866f,  0.951056f,  0.162460f),
            vm::vec3f(-0.500000f,  0.809017f,  0.309017f),
            vm::vec3f(-0.238856f,  0.864188f, -0.442863f),
            vm::vec3f(-0.262866f,  0.951056f, -0.162460f),
            vm::vec3f(-0.500000f,  0.809017f, -0.309017f),
            vm::vec3f(-0.850651f,  0.525731f,  0.000000f),
            vm::vec3f(-0.716567f,  0.681718f,  0.147621f),
            vm::vec3f(-0.716567f,  0.681718f, -0.147621f),
            vm::vec3f(-0.525731f,  0.850651f,  0.000000f),
            vm::vec3f(-0.425325f,  0.688191f,  0.587785f),
            vm::vec3f(-0.864188f,  0.442863f,  0.238856f),
            vm::vec3f(-0.688191f,  0.587785f,  0.425325f),
            vm::vec3f(-0.809017f,  0.309017f,  0.500000f),
            vm::vec3f(-0.681718f,  0.147621f,  0.716567f),
            vm::vec3f(-0.587785f,  0.425325f,  0.688191f),
            vm::vec3f(-0.955423f,  0.295242f,  0.000000f),
            vm::vec3f( 1.000000f,  0.000000f,  0.000000f),
            vm::vec3f(-0.951056f,  0.162460f,  0.262866f),
            vm::vec3f(-0.850651f, -0.525731f,  0.000000f),
            vm::vec3f(-0.955423f, -0.295242f,  0.000000f),
            vm::vec3f(-0.864188f, -0.442863f,  0.238856f),
            vm::vec3f(-0.951056f, -0.162460f,  0.262866f),
            vm::vec3f(-0.809017f, -0.309017f,  0.500000f),
            vm::vec3f(-0.681718f, -0.147621f,  0.716567f),
            vm::vec3f(-0.850651f,  0.000000f,  0.525731f),
            vm::vec3f(-0.864188f,  0.442863f, -0.238856f),
            vm::vec3f(-0.809017f,  0.309017f, -0.500000f),
            vm::vec3f(-0.951056f,  0.162460f, -0.262866f),
            vm::vec3f(-0.525731f,  0.000000f, -0.850651f),
            vm::vec3f(-0.681718f,  0.147621f, -0.716567f),
            vm::vec3f(-0.681718f, -0.147621f, -0.716567f),
            vm::vec3f(-0.850651f,  0.000000f, -0.525731f),
            vm::vec3f(-0.809017f, -0.309017f, -0.500000f),
            vm::vec3f(-0.864188f, -0.442863f, -0.238856f),
            vm::vec3f(-0.951056f, -0.162460f, -0.262866f),
            vm::vec3f(-0.147621f,  0.716567f, -0.681718f),
            vm::vec3f(-0.309017f,  0.500000f, -0.809017f),
            vm::vec3f(-0.425325f,  0.688191f, -0.587785f),
            vm::vec3f(-0.442863f,  0.238856f, -0.864188f),
            vm::vec3f(-0.587785f,  0.425325f, -0.688191f),
            vm::vec3f(-0.688191f,  0.587785f, -0.425325f),
            vm::vec3f(-0.147621f,  0.716567f, -0.681718f),
            vm::vec3f(-0.309017f,  0.500000f, -0.809017f),
            vm::vec3f(-0.000000f,  0.525731f, -0.850651f),
            vm::vec3f(-0.525731f,  0.000000f, -0.850651f),
            vm::vec3f(-0.442863f,  0.238856f, -0.864188f),
            vm::vec3f(-0.295242f,  0.000000f, -0.955423f),
            vm::vec3f(-0.162460f,  0.262866f, -0.951056f),
            vm::vec3f(-0.000000f,  0.000000f, -1.000000f),
            vm::vec3f(-0.295242f,  0.000000f, -0.955423f),
            vm::vec3f(-0.162460f,  0.262866f, -0.951056f),
            vm::vec3f(-0.442863f, -0.238856f, -0.864188f),
            vm::vec3f(-0.309017f, -0.500000f, -0.809017f),
            vm::vec3f(-0.162460f, -0.262866f, -0.951056f),
            vm::vec3f(-0.000000f, -0.850651f, -0.525731f),
            vm::vec3f(-0.147621f, -0.716567f, -0.681718f),
            vm::vec3f(-0.147621f, -0.716567f, -0.681718f),
            vm::vec3f(-0.000000f, -0.525731f, -0.850651f),
            vm::vec3f(-0.309017f, -0.500000f, -0.809017f),
            vm::vec3f(-0.442863f, -0.238856f, -0.864188f),
            vm::vec3f(-0.162460f, -0.262866f, -0.951056f),
            vm::vec3f(-0.238856f, -0.864188f, -0.442863f),
            vm::vec3f(-0.500000f, -0.809017f, -0.309017f),
            vm::vec3f(-0.425325f, -0.688191f, -0.587785f),
            vm::vec3f(-0.716567f, -0.681718f, -0.147621f),
            vm::vec3f(-0.688191f, -0.587785f, -0.425325f),
            vm::vec3f(-0.587785f, -0.425325f, -0.688191f),
            vm::vec3f(-0.000000f, -0.955423f, -0.295242f),
            vm::vec3f(-0.000000f, -1.000000f,  0.000000f),
            vm::vec3f(-0.262866f, -0.951056f, -0.162460f),
            vm::vec3f(-0.000000f, -0.850651f,  0.525731f),
            vm::vec3f(-0.000000f, -0.955423f,  0.295242f),
            vm::vec3f(-0.238856f, -0.864188f,  0.442863f),
            vm::vec3f(-0.262866f, -0.951056f,  0.162460f),
            vm::vec3f(-0.500000f, -0.809017f,  0.309017f),
            vm::vec3f(-0.716567f, -0.681718f,  0.147621f),
            vm::vec3f(-0.525731f, -0.850651f,  0.000000f),
            vm::vec3f(-0.238856f, -0.864188f, -0.442863f),
            vm::vec3f(-0.500000f, -0.809017f, -0.309017f),
            vm::vec3f(-0.262866f, -0.951056f, -0.162460f),
            vm::vec3f(-0.850651f, -0.525731f,  0.000000f),
            vm::vec3f(-0.716567f, -0.681718f, -0.147621f),
            vm::vec3f(-0.716567f, -0.681718f,  0.147621f),
            vm::vec3f(-0.525731f, -0.850651f,  0.000000f),
            vm::vec3f(-0.500000f, -0.809017f,  0.309017f),
            vm::vec3f(-0.238856f, -0.864188f,  0.442863f),
            vm::vec3f(-0.262866f, -0.951056f,  0.162460f),
            vm::vec3f(-0.864188f, -0.442863f,  0.238856f),
            vm::vec3f(-0.809017f, -0.309017f,  0.500000f),
            vm::vec3f(-0.688191f, -0.587785f,  0.425325f),
            vm::vec3f(-0.681718f, -0.147621f,  0.716567f),
            vm::vec3f(-0.442863f, -0.238856f,  0.864188f),
            vm::vec3f(-0.587785f, -0.425325f,  0.688191f),
            vm::vec3f(-0.309017f, -0.500000f,  0.809017f),
            vm::vec3f(-0.147621f, -0.716567f,  0.681718f),
            vm::vec3f(-0.425325f, -0.688191f,  0.587785f),
            vm::vec3f(-0.162460f, -0.262866f,  0.951056f),
            vm::vec3f(-0.442863f, -0.238856f,  0.864188f),
            vm::vec3f(-0.162460f, -0.262866f,  0.951056f),
            vm::vec3f(-0.309017f, -0.500000f,  0.809017f),
            vm::vec3f(-0.147621f, -0.716567f,  0.681718f),
            vm::vec3f(-0.000000f, -0.525731f,  0.850651f),
            vm::vec3f(-0.425325f, -0.688191f,  0.587785f),
            vm::vec3f(-0.587785f, -0.425325f,  0.688191f),
            vm::vec3f(-0.688191f, -0.587785f,  0.425325f),
            vm::vec3f(-0.955423f,  0.295242f,  0.000000f),
            vm::vec3f(-0.951056f,  0.162460f,  0.262866f),
            vm::vec3f(-1.000000f,  0.000000f,  0.000000f),
            vm::vec3f(-0.850651f,  0.000000f,  0.525731f),
            vm::vec3f(-0.955423f, -0.295242f,  0.000000f),
            vm::vec3f(-0.951056f, -0.162460f,  0.262866f),
            vm::vec3f(-0.864188f,  0.442863f, -0.238856f),
            vm::vec3f(-0.951056f,  0.162460f, -0.262866f),
            vm::vec3f(-0.809017f,  0.309017f, -0.500000f),
            vm::vec3f(-0.864188f, -0.442863f, -0.238856f),
            vm::vec3f(-0.951056f, -0.162460f, -0.262866f),
            vm::vec3f(-0.809017f, -0.309017f, -0.500000f),
            vm::vec3f(-0.681718f,  0.147621f, -0.716567f),
            vm::vec3f(-0.681718f, -0.147621f, -0.716567f),
            vm::vec3f(-0.850651f,  0.000000f, -0.525731f),
            vm::vec3f(-0.688191f,  0.587785f, -0.425325f),
            vm::vec3f(-0.587785f,  0.425325f, -0.688191f),
            vm::vec3f(-0.425325f,  0.688191f, -0.587785f),
            vm::vec3f(-0.425325f, -0.688191f, -0.587785f),
            vm::vec3f(-0.587785f, -0.425325f, -0.688191f),
            vm::vec3f(-0.688191f, -0.587785f, -0.425325f),
        };

        static const int MF_HOLEY = (1 << 14);

        MdlParser::MdlParser(const std::string& name, const char* begin, const char* end, const Assets::Palette& palette) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_palette(palette) {
            assert(m_begin < m_end);
            unused(m_end);
        }

        std::unique_ptr<Assets::EntityModel> MdlParser::doInitializeModel(Logger& /* logger */) {
            auto reader = Reader::from(m_begin, m_end);

            const auto ident = reader.readInt<int32_t>();
            const auto version = reader.readInt<int32_t>();

            if (ident != MdlLayout::Ident) {
                throw AssetException("Unknown MDL model ident: " + std::to_string(ident));
            }
            if (version != MdlLayout::Version6) {
                throw AssetException("Unknown MDL model version: " + std::to_string(version));
            }

            /* const auto scale = */ reader.readVec<float, 3>();
            /* const auto origin = */ reader.readVec<float, 3>();

            reader.seekFromBegin(MdlLayout::HeaderNumSkins);
            const auto skinCount = reader.readSize<int32_t>();
            const auto skinWidth = reader.readSize<int32_t>();
            const auto skinHeight = reader.readSize<int32_t>();
            /* const auto skinVertexCount = */ reader.readSize<int32_t>();
            /* const auto skinTriangleCount = */ reader.readSize<int32_t>();
            const auto frameCount = reader.readSize<int32_t>();
            /* const auto syncType = */ reader.readSize<int32_t>();
            const auto flags = reader.readInt<int32_t>();

            auto model = std::make_unique<Assets::EntityModel>(m_name, Assets::PitchType::MdlInverted);
            model->addFrames(frameCount);
            auto& surface = model->addSurface(m_name);

            reader.seekFromBegin(MdlLayout::Skins);
            parseSkins(reader, surface, skinCount, skinWidth, skinHeight, flags);

            return model;
        }

        void MdlParser::doLoadFrame(const size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */) {
            auto reader = Reader::from(m_begin, m_end);

            const auto ident = reader.readInt<int32_t>();
            const auto version = reader.readInt<int32_t>();

            if (ident != MdlLayout::Ident) {
                throw AssetException("Unknown MDL model ident: " + std::to_string(ident));
            }
            if (version != MdlLayout::Version6) {
                throw AssetException("Unknown MDL model version: " + std::to_string(version));
            }

            const auto scale = reader.readVec<float, 3>();
            const auto origin = reader.readVec<float, 3>();

            reader.seekFromBegin(MdlLayout::HeaderNumSkins);
            const auto skinCount = reader.readSize<int32_t>();
            const auto skinWidth = reader.readSize<int32_t>();
            const auto skinHeight = reader.readSize<int32_t>();
            const auto vertexCount = reader.readSize<int32_t>();
            const auto triangleCount = reader.readSize<int32_t>();
            /* const auto frameCount = */ reader.readSize<int32_t>();
            /* const auto syncType = */ reader.readSize<int32_t>();
            const auto flags = reader.readInt<int32_t>();

            reader.seekFromBegin(MdlLayout::Skins);
            skipSkins(reader, skinCount, skinWidth, skinHeight, flags);

            const auto vertices = parseVertices(reader, vertexCount);
            const auto triangles = parseTriangles(reader, triangleCount);

            auto& surface = model.surface(0);
            skipFrames(reader, frameIndex, vertexCount);
            parseFrame(reader, model, frameIndex, surface, triangles, vertices, skinWidth, skinHeight, origin, scale);
        }

        void MdlParser::parseSkins(Reader& reader, Assets::EntityModelSurface& surface, const size_t count, const size_t width, const size_t height, const int flags) {
            const auto size = width * height;
            const auto transparency = (flags & MF_HOLEY)
                    ? Assets::PaletteTransparency::Index255Transparent
                    : Assets::PaletteTransparency::Opaque;
            const auto type = (transparency == Assets::PaletteTransparency::Index255Transparent)
                              ? Assets::TextureType::Masked
                              : Assets::TextureType::Opaque;
            Color avgColor;
            std::vector<Assets::Texture> textures;
            textures.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                const auto skinGroup = reader.readSize<int32_t>();
                if (skinGroup == 0) {
                    Assets::TextureBuffer rgbaImage(size * 4);
                    m_palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);

                    const std::string textureName = m_name + "_" + kdl::str_to_string(i);
                    textures.emplace_back(textureName, width, height, avgColor, std::move(rgbaImage), GL_RGBA, type);
                } else {
                    const auto pictureCount = reader.readSize<int32_t>();

                    Assets::TextureBuffer rgbaImage(size * 4);
                    reader.seekForward(pictureCount * 4); // skip the picture times

                    m_palette.indexedToRgba(reader, size, rgbaImage, transparency, avgColor);
                    reader.seekForward((pictureCount - 1) * size);  // skip all remaining pictures

                    const std::string textureName = m_name + "_" + kdl::str_to_string(i);
                    textures.emplace_back(textureName, width, height, avgColor, std::move(rgbaImage), GL_RGBA, type);
                }
            }
            
            surface.setSkins(std::move(textures));
        }

        void MdlParser::skipSkins(Reader& reader, const size_t count, const size_t width, const size_t height, const int /* flags */) {
            const auto size = width * height;

            for (size_t i = 0; i < count; ++i) {
                const auto skinGroup = reader.readSize<int32_t>();
                if (skinGroup == 0) {
                    reader.seekForward(size);
                } else {
                    const auto pictureCount = reader.readSize<int32_t>();
                    reader.seekForward(pictureCount * 4); // skip the picture times
                    reader.seekForward(pictureCount * size);  // skip all pictures
                }
            }
        }

        MdlParser::MdlSkinVertexList MdlParser::parseVertices(Reader& reader, size_t count) {
            MdlSkinVertexList vertices(count);
            for (size_t i = 0; i < count; ++i) {
                vertices[i].onseam = reader.readBool<int32_t>();
                vertices[i].s = reader.readInt<int32_t>();
                vertices[i].t = reader.readInt<int32_t>();
            }
            return vertices;
        }

        MdlParser::MdlSkinTriangleList MdlParser::parseTriangles(Reader& reader, size_t count) {
            MdlSkinTriangleList triangles(count);
            for (size_t i = 0; i < count; ++i) {
                triangles[i].front = reader.readBool<int32_t>();
                for (size_t j = 0; j < 3; ++j) {
                    triangles[i].vertices[j] = reader.readSize<int32_t>();
                }
            }
            return triangles;
        }

        void MdlParser::skipFrames(Reader& reader, const size_t count, size_t vertexCount) {
            const auto frameLength = MdlLayout::SimpleFrameName + MdlLayout::SimpleFrameLength + vertexCount * 4;

            for (size_t i = 0; i < count; ++i) {
                const auto type = reader.readInt<int32_t>();
                if (type == 0) { // single frame
                    reader.seekForward(frameLength);
                } else { // frame group, but we only read the first frame
                    const auto groupFrameCount = reader.readSize<int32_t>();
                    reader.seekBackward(sizeof(int32_t));

                    const auto frameTimeLength = MdlLayout::MultiFrameTimes + groupFrameCount * sizeof(float);

                    // forward to after the last group frame as if we had read them
                    reader.seekForward(frameTimeLength + groupFrameCount * frameLength);
                }
            }
        }

        void MdlParser::parseFrame(Reader& reader, Assets::EntityModel& model, size_t frameIndex, Assets::EntityModelSurface& surface, const MdlSkinTriangleList& triangles, const MdlSkinVertexList& vertices, size_t skinWidth, size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale) {
            const auto frameLength = MdlLayout::SimpleFrameName + MdlLayout::SimpleFrameLength + vertices.size() * 4;

            const auto type = reader.readInt<int32_t>();
            if (type == 0) { // single frame
                doParseFrame(reader.subReaderFromCurrent(frameLength), model, frameIndex, surface, triangles, vertices, skinWidth, skinHeight, origin, scale);
            } else { // frame group, but we only read the first frame
                const auto groupFrameCount = reader.readSize<int32_t>();
                reader.seekBackward(sizeof(int32_t));

                const auto frameTimeLength = MdlLayout::MultiFrameTimes + groupFrameCount * sizeof(float);
                doParseFrame(reader.subReaderFromCurrent(frameTimeLength, frameLength), model, frameIndex, surface, triangles, vertices, skinWidth, skinHeight, origin, scale);
            }
        }

        void MdlParser::doParseFrame(Reader reader, Assets::EntityModel& model, size_t frameIndex, Assets::EntityModelSurface& surface, const MdlSkinTriangleList& triangles, const MdlSkinVertexList& vertices, const size_t skinWidth, const size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale) {
            reader.seekForward(MdlLayout::SimpleFrameName);
            const auto name = reader.readString(MdlLayout::SimpleFrameLength);

            PackedFrameVertexList packedVertices(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    packedVertices[i][j] = reader.readUnsignedChar<char>();
                }
            }

            std::vector<vm::vec3f> positions(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i) {
                positions[i] = unpackFrameVertex(packedVertices[i], origin, scale);
            }

            vm::bbox3f::builder bounds;

            std::vector<Assets::EntityModelVertex> frameTriangles;
            frameTriangles.reserve(triangles.size());
            for (size_t i = 0; i < triangles.size(); ++i) {
                const auto& triangle = triangles[i];
                for (size_t j = 0; j < 3; ++j) {
                    const auto vertexIndex = triangle.vertices[j];
                    const auto& skinVertex = vertices[vertexIndex];

                    auto texCoords = vm::vec2f(float(skinVertex.s) / float(skinWidth), float(skinVertex.t) / float(skinHeight));
                    if (skinVertex.onseam && !triangle.front) {
                        texCoords[0] += 0.5f;
                    }

                    const auto& position = positions[vertexIndex];
                    bounds.add(position);
                    frameTriangles.emplace_back(position, texCoords);
                }
            }

            Renderer::IndexRangeMap::Size size;
            size.inc(Renderer::PrimType::Triangles, frameTriangles.size());

            Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder(frameTriangles.size() * 3, size);
            builder.addTriangles(frameTriangles);

            auto& frame = model.loadFrame(frameIndex, name, bounds.bounds());
            surface.addIndexedMesh(frame, builder.vertices(), builder.indices());
        }

        vm::vec3f MdlParser::unpackFrameVertex(const PackedFrameVertex& vertex, const vm::vec3f& origin, const vm::vec3f& scale) const {
            vm::vec3f result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = origin[i] + scale[i] * static_cast<float>(vertex[i]);
            }
            return result;
        }
    }
}
