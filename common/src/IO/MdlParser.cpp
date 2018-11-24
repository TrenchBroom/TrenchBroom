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

#include "CollectionUtils.h"
#include "Macros.h"
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "Assets/Palette.h"
#include "IO/IOUtils.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"

#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        namespace MdlLayout {
            static const unsigned int HeaderScale       = 0x8;
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
        
        MdlParser::MdlParser(const String& name, const char* begin, const char* end, const Assets::Palette& palette) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_palette(palette) {
            assert(m_begin < m_end);
            unused(m_end);
        }

        Assets::EntityModel* MdlParser::doParseModel() {
            const auto* cursor = m_begin + MdlLayout::HeaderScale;
            const auto scale = readVec3f(cursor);
            const auto origin = readVec3f(cursor);
            
            cursor = m_begin + MdlLayout::HeaderNumSkins;
            const auto skinCount = readSize<int32_t>(cursor);
            const auto skinWidth = readSize<int32_t>(cursor);
            const auto skinHeight = readSize<int32_t>(cursor);
            const auto skinVertexCount = readSize<int32_t>(cursor);
            const auto skinTriangleCount = readSize<int32_t>(cursor);
            const auto frameCount = readSize<int32_t>(cursor);
            /* const auto syncType = */ readSize<int32_t>(cursor);
            const auto flags = readInt<int32_t>(cursor);

            auto model = std::make_unique<Assets::EntityModel>(m_name);

            parseSkins(cursor, model.get(), skinCount, skinWidth, skinHeight, flags);

            const auto skinVertices = parseSkinVertices(cursor, skinVertexCount);
            const auto skinTriangles = parseSkinTriangles(cursor, skinTriangleCount);

            parseFrames(cursor, model.get(), frameCount, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale);

            return model.release();
        }

        void MdlParser::parseSkins(const char*& cursor, Assets::EntityModel* model, const size_t count, const size_t width, const size_t height, const int flags) {
            const auto size = width * height;
            const auto transparency = (flags & MF_HOLEY)
                    ? Assets::PaletteTransparency::Index255Transparent
                    : Assets::PaletteTransparency::Opaque;
            const auto type = (transparency == Assets::PaletteTransparency::Index255Transparent)
                              ? Assets::TextureType::Masked
                              : Assets::TextureType::Opaque;
            Color avgColor;
            StringStream textureName;

            cursor = m_begin + MdlLayout::Skins;
            for (size_t i = 0; i < count; ++i) {
                const auto skinGroup = readSize<int32_t>(cursor);
                if (skinGroup == 0) {
                    Buffer<unsigned char> rgbaImage(size * 4);
                    m_palette.indexedToRgba(cursor, size, rgbaImage, transparency, avgColor);
                    cursor += size;

                    textureName << m_name << "_" << i;

                    model->addSkin(new Assets::Texture(textureName.str(), width, height, avgColor, rgbaImage, GL_RGBA, type));
                } else {
                    const auto pictureCount = readSize<int32_t>(cursor);

                    Buffer<unsigned char> rgbaImage(size * 4);
                    cursor += pictureCount * 4; // skip the picture times

                    m_palette.indexedToRgba(cursor, size, rgbaImage, transparency, avgColor);
                    cursor += pictureCount * size; // skip all pictures

                    textureName << m_name << "_" << i;

                    model->addSkin(new Assets::Texture(textureName.str(), width, height, avgColor, rgbaImage, GL_RGBA, type));
                }
            }
        }

        MdlParser::MdlSkinVertexList MdlParser::parseSkinVertices(const char*& cursor, const size_t count) {
            MdlSkinVertexList vertices(count);
            for (size_t i = 0; i < count; ++i) {
                vertices[i].onseam = readBool<int32_t>(cursor);
                vertices[i].s = readInt<int32_t>(cursor);
                vertices[i].t = readInt<int32_t>(cursor);
            }
            return vertices;
        }

        MdlParser::MdlSkinTriangleList MdlParser::parseSkinTriangles(const char*& cursor, const size_t count) {
            MdlSkinTriangleList triangles(count);
            for (size_t i = 0; i < count; ++i) {
                triangles[i].front = readBool<int32_t>(cursor);
                for (size_t j = 0; j < 3; ++j) {
                    triangles[i].vertices[j] = readSize<int32_t>(cursor);
                }
            }
            return triangles;
        }

        void MdlParser::parseFrames(const char*& cursor, Assets::EntityModel* model, const size_t count, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, const size_t skinWidth, const size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale) {
            for (size_t i = 0; i < count; ++i) {
                const auto type = readInt<int32_t>(cursor);
                if (type == 0) { // single frame
                    parseFrame(cursor, model, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale);
                } else { // frame group, but we only read the first frame
                    const auto* base = cursor;
                    const auto groupFrameCount = readSize<int32_t>(cursor);

                    const auto* frameCursor = base + MdlLayout::MultiFrameTimes + groupFrameCount * sizeof(float);
                    parseFrame(frameCursor, model, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale);

                    // forward to after the last group frame as if we had read them all
                    const auto offset = (groupFrameCount - 1) * (MdlLayout::SimpleFrameName + MdlLayout::SimpleFrameLength + skinVertices.size() * 4);
                    cursor = frameCursor + offset;
                }
            }
        }

        void MdlParser::parseFrame(const char*& cursor, Assets::EntityModel* model, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, const size_t skinWidth, const size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale) {
            using Vertex = Assets::EntityModel::Vertex;
            using VertexList = Vertex::List;

            char name[MdlLayout::SimpleFrameLength + 1];
            name[MdlLayout::SimpleFrameLength] = 0;
            cursor += MdlLayout::SimpleFrameName;
            readBytes(cursor, name, MdlLayout::SimpleFrameLength);
            
            PackedFrameVertexList packedVertices(skinVertices.size());
            for (size_t i = 0; i < skinVertices.size(); ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    packedVertices[i][j] = static_cast<unsigned char>(*cursor++);
                }
            }

            std::vector<vm::vec3f> positions(skinVertices.size());
            for (size_t i = 0; i < skinVertices.size(); ++i) {
                positions[i] = unpackFrameVertex(packedVertices[i], origin, scale);
            }

            VertexList frameTriangles;
            frameTriangles.reserve(skinTriangles.size());
            for (size_t i = 0; i < skinTriangles.size(); ++i) {
                const auto& triangle = skinTriangles[i];
                for (size_t j = 0; j < 3; ++j) {
                    const auto vertexIndex = triangle.vertices[j];
                    const auto& skinVertex = skinVertices[vertexIndex];

                    auto texCoords = vm::vec2f(float(skinVertex.s) / float(skinWidth), float(skinVertex.t) / float(skinHeight));
                    if (skinVertex.onseam && !triangle.front) {
                        texCoords[0] += 0.5f;
                    }

                    frameTriangles.push_back(Vertex(positions[vertexIndex], texCoords));
                }
            }

            Renderer::IndexRangeMap::Size size;
            size.inc(GL_TRIANGLES, frameTriangles.size());

            Renderer::IndexRangeMapBuilder<Assets::EntityModel::Vertex::Spec> builder(frameTriangles.size() * 3, size);
            builder.addTriangles(frameTriangles);

            model->addFrame(String(name), builder.vertices(), builder.indexArray());
        }

        vm::vec3f MdlParser::unpackFrameVertex(const PackedFrameVertex& vertex, const vm::vec3f& origin, const vm::vec3f& scale) const {
            vm::vec3f result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = origin[i] + scale[i]*static_cast<float>(vertex[i]);
            }
            return result;
        }
    }
}
