/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Assets/Texture.h"
#include "Assets/MdlModel.h"
#include "Assets/Palette.h"
#include "IO/IOUtils.h"

#include <cassert>

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

        const Vec3f MdlParser::Normals[] = {
            Vec3f(-0.525731f,  0.000000f,  0.850651f),
            Vec3f(-0.442863f,  0.238856f,  0.864188f),
            Vec3f(-0.295242f,  0.000000f,  0.955423f),
            Vec3f(-0.309017f,  0.500000f,  0.809017f),
            Vec3f(-0.162460f,  0.262866f,  0.951056f),
            Vec3f(-0.000000f,  0.000000f,  1.000000f),
            Vec3f(-0.000000f,  0.850651f,  0.525731f),
            Vec3f(-0.147621f,  0.716567f,  0.681718f),
            Vec3f(-0.147621f,  0.716567f,  0.681718f),
            Vec3f(-0.000000f,  0.525731f,  0.850651f),
            Vec3f(-0.309017f,  0.500000f,  0.809017f),
            Vec3f(-0.525731f,  0.000000f,  0.850651f),
            Vec3f(-0.295242f,  0.000000f,  0.955423f),
            Vec3f(-0.442863f,  0.238856f,  0.864188f),
            Vec3f(-0.162460f,  0.262866f,  0.951056f),
            Vec3f(-0.681718f,  0.147621f,  0.716567f),
            Vec3f(-0.809017f,  0.309017f,  0.500000f),
            Vec3f(-0.587785f,  0.425325f,  0.688191f),
            Vec3f(-0.850651f,  0.525731f,  0.000000f),
            Vec3f(-0.864188f,  0.442863f,  0.238856f),
            Vec3f(-0.716567f,  0.681718f,  0.147621f),
            Vec3f(-0.688191f,  0.587785f,  0.425325f),
            Vec3f(-0.500000f,  0.809017f,  0.309017f),
            Vec3f(-0.238856f,  0.864188f,  0.442863f),
            Vec3f(-0.425325f,  0.688191f,  0.587785f),
            Vec3f(-0.716567f,  0.681718f, -0.147621f),
            Vec3f(-0.500000f,  0.809017f, -0.309017f),
            Vec3f(-0.525731f,  0.850651f,  0.000000f),
            Vec3f(-0.000000f,  0.850651f, -0.525731f),
            Vec3f(-0.238856f,  0.864188f, -0.442863f),
            Vec3f(-0.000000f,  0.955423f, -0.295242f),
            Vec3f(-0.262866f,  0.951056f, -0.162460f),
            Vec3f(-0.000000f,  1.000000f,  0.000000f),
            Vec3f(-0.000000f,  0.955423f,  0.295242f),
            Vec3f(-0.262866f,  0.951056f,  0.162460f),
            Vec3f(-0.238856f,  0.864188f,  0.442863f),
            Vec3f(-0.262866f,  0.951056f,  0.162460f),
            Vec3f(-0.500000f,  0.809017f,  0.309017f),
            Vec3f(-0.238856f,  0.864188f, -0.442863f),
            Vec3f(-0.262866f,  0.951056f, -0.162460f),
            Vec3f(-0.500000f,  0.809017f, -0.309017f),
            Vec3f(-0.850651f,  0.525731f,  0.000000f),
            Vec3f(-0.716567f,  0.681718f,  0.147621f),
            Vec3f(-0.716567f,  0.681718f, -0.147621f),
            Vec3f(-0.525731f,  0.850651f,  0.000000f),
            Vec3f(-0.425325f,  0.688191f,  0.587785f),
            Vec3f(-0.864188f,  0.442863f,  0.238856f),
            Vec3f(-0.688191f,  0.587785f,  0.425325f),
            Vec3f(-0.809017f,  0.309017f,  0.500000f),
            Vec3f(-0.681718f,  0.147621f,  0.716567f),
            Vec3f(-0.587785f,  0.425325f,  0.688191f),
            Vec3f(-0.955423f,  0.295242f,  0.000000f),
            Vec3f( 1.000000f,  0.000000f,  0.000000f),
            Vec3f(-0.951056f,  0.162460f,  0.262866f),
            Vec3f(-0.850651f, -0.525731f,  0.000000f),
            Vec3f(-0.955423f, -0.295242f,  0.000000f),
            Vec3f(-0.864188f, -0.442863f,  0.238856f),
            Vec3f(-0.951056f, -0.162460f,  0.262866f),
            Vec3f(-0.809017f, -0.309017f,  0.500000f),
            Vec3f(-0.681718f, -0.147621f,  0.716567f),
            Vec3f(-0.850651f,  0.000000f,  0.525731f),
            Vec3f(-0.864188f,  0.442863f, -0.238856f),
            Vec3f(-0.809017f,  0.309017f, -0.500000f),
            Vec3f(-0.951056f,  0.162460f, -0.262866f),
            Vec3f(-0.525731f,  0.000000f, -0.850651f),
            Vec3f(-0.681718f,  0.147621f, -0.716567f),
            Vec3f(-0.681718f, -0.147621f, -0.716567f),
            Vec3f(-0.850651f,  0.000000f, -0.525731f),
            Vec3f(-0.809017f, -0.309017f, -0.500000f),
            Vec3f(-0.864188f, -0.442863f, -0.238856f),
            Vec3f(-0.951056f, -0.162460f, -0.262866f),
            Vec3f(-0.147621f,  0.716567f, -0.681718f),
            Vec3f(-0.309017f,  0.500000f, -0.809017f),
            Vec3f(-0.425325f,  0.688191f, -0.587785f),
            Vec3f(-0.442863f,  0.238856f, -0.864188f),
            Vec3f(-0.587785f,  0.425325f, -0.688191f),
            Vec3f(-0.688191f,  0.587785f, -0.425325f),
            Vec3f(-0.147621f,  0.716567f, -0.681718f),
            Vec3f(-0.309017f,  0.500000f, -0.809017f),
            Vec3f(-0.000000f,  0.525731f, -0.850651f),
            Vec3f(-0.525731f,  0.000000f, -0.850651f),
            Vec3f(-0.442863f,  0.238856f, -0.864188f),
            Vec3f(-0.295242f,  0.000000f, -0.955423f),
            Vec3f(-0.162460f,  0.262866f, -0.951056f),
            Vec3f(-0.000000f,  0.000000f, -1.000000f),
            Vec3f(-0.295242f,  0.000000f, -0.955423f),
            Vec3f(-0.162460f,  0.262866f, -0.951056f),
            Vec3f(-0.442863f, -0.238856f, -0.864188f),
            Vec3f(-0.309017f, -0.500000f, -0.809017f),
            Vec3f(-0.162460f, -0.262866f, -0.951056f),
            Vec3f(-0.000000f, -0.850651f, -0.525731f),
            Vec3f(-0.147621f, -0.716567f, -0.681718f),
            Vec3f(-0.147621f, -0.716567f, -0.681718f),
            Vec3f(-0.000000f, -0.525731f, -0.850651f),
            Vec3f(-0.309017f, -0.500000f, -0.809017f),
            Vec3f(-0.442863f, -0.238856f, -0.864188f),
            Vec3f(-0.162460f, -0.262866f, -0.951056f),
            Vec3f(-0.238856f, -0.864188f, -0.442863f),
            Vec3f(-0.500000f, -0.809017f, -0.309017f),
            Vec3f(-0.425325f, -0.688191f, -0.587785f),
            Vec3f(-0.716567f, -0.681718f, -0.147621f),
            Vec3f(-0.688191f, -0.587785f, -0.425325f),
            Vec3f(-0.587785f, -0.425325f, -0.688191f),
            Vec3f(-0.000000f, -0.955423f, -0.295242f),
            Vec3f(-0.000000f, -1.000000f,  0.000000f),
            Vec3f(-0.262866f, -0.951056f, -0.162460f),
            Vec3f(-0.000000f, -0.850651f,  0.525731f),
            Vec3f(-0.000000f, -0.955423f,  0.295242f),
            Vec3f(-0.238856f, -0.864188f,  0.442863f),
            Vec3f(-0.262866f, -0.951056f,  0.162460f),
            Vec3f(-0.500000f, -0.809017f,  0.309017f),
            Vec3f(-0.716567f, -0.681718f,  0.147621f),
            Vec3f(-0.525731f, -0.850651f,  0.000000f),
            Vec3f(-0.238856f, -0.864188f, -0.442863f),
            Vec3f(-0.500000f, -0.809017f, -0.309017f),
            Vec3f(-0.262866f, -0.951056f, -0.162460f),
            Vec3f(-0.850651f, -0.525731f,  0.000000f),
            Vec3f(-0.716567f, -0.681718f, -0.147621f),
            Vec3f(-0.716567f, -0.681718f,  0.147621f),
            Vec3f(-0.525731f, -0.850651f,  0.000000f),
            Vec3f(-0.500000f, -0.809017f,  0.309017f),
            Vec3f(-0.238856f, -0.864188f,  0.442863f),
            Vec3f(-0.262866f, -0.951056f,  0.162460f),
            Vec3f(-0.864188f, -0.442863f,  0.238856f),
            Vec3f(-0.809017f, -0.309017f,  0.500000f),
            Vec3f(-0.688191f, -0.587785f,  0.425325f),
            Vec3f(-0.681718f, -0.147621f,  0.716567f),
            Vec3f(-0.442863f, -0.238856f,  0.864188f),
            Vec3f(-0.587785f, -0.425325f,  0.688191f),
            Vec3f(-0.309017f, -0.500000f,  0.809017f),
            Vec3f(-0.147621f, -0.716567f,  0.681718f),
            Vec3f(-0.425325f, -0.688191f,  0.587785f),
            Vec3f(-0.162460f, -0.262866f,  0.951056f),
            Vec3f(-0.442863f, -0.238856f,  0.864188f),
            Vec3f(-0.162460f, -0.262866f,  0.951056f),
            Vec3f(-0.309017f, -0.500000f,  0.809017f),
            Vec3f(-0.147621f, -0.716567f,  0.681718f),
            Vec3f(-0.000000f, -0.525731f,  0.850651f),
            Vec3f(-0.425325f, -0.688191f,  0.587785f),
            Vec3f(-0.587785f, -0.425325f,  0.688191f),
            Vec3f(-0.688191f, -0.587785f,  0.425325f),
            Vec3f(-0.955423f,  0.295242f,  0.000000f),
            Vec3f(-0.951056f,  0.162460f,  0.262866f),
            Vec3f(-1.000000f,  0.000000f,  0.000000f),
            Vec3f(-0.850651f,  0.000000f,  0.525731f),
            Vec3f(-0.955423f, -0.295242f,  0.000000f),
            Vec3f(-0.951056f, -0.162460f,  0.262866f),
            Vec3f(-0.864188f,  0.442863f, -0.238856f),
            Vec3f(-0.951056f,  0.162460f, -0.262866f),
            Vec3f(-0.809017f,  0.309017f, -0.500000f),
            Vec3f(-0.864188f, -0.442863f, -0.238856f),
            Vec3f(-0.951056f, -0.162460f, -0.262866f),
            Vec3f(-0.809017f, -0.309017f, -0.500000f),
            Vec3f(-0.681718f,  0.147621f, -0.716567f),
            Vec3f(-0.681718f, -0.147621f, -0.716567f),
            Vec3f(-0.850651f,  0.000000f, -0.525731f),
            Vec3f(-0.688191f,  0.587785f, -0.425325f),
            Vec3f(-0.587785f,  0.425325f, -0.688191f),
            Vec3f(-0.425325f,  0.688191f, -0.587785f),
            Vec3f(-0.425325f, -0.688191f, -0.587785f),
            Vec3f(-0.587785f, -0.425325f, -0.688191f),
            Vec3f(-0.688191f, -0.587785f, -0.425325f),
        };

        
        MdlParser::MdlParser(const String& name, const char* begin, const char* end, const Assets::Palette& palette) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_palette(palette) {
            assert(begin < end);
            unused(m_end);
        }

        Assets::EntityModel* MdlParser::doParseModel() {
            Assets::MdlModel* model = new Assets::MdlModel(m_name);
            
            const char* cursor = m_begin + MdlLayout::HeaderScale;
            const Vec3f scale = readVec3f(cursor);
            const Vec3f origin = readVec3f(cursor);
            
            cursor = m_begin + MdlLayout::HeaderNumSkins;
            const size_t skinCount = readSize<int32_t>(cursor);
            const size_t skinWidth = readSize<int32_t>(cursor);
            const size_t skinHeight = readSize<int32_t>(cursor);
            const size_t skinVertexCount = readSize<int32_t>(cursor);
            const size_t skinTriangleCount = readSize<int32_t>(cursor);
            const size_t frameCount = readSize<int32_t>(cursor);
            
            parseSkins(cursor, *model, skinCount, skinWidth, skinHeight);
            const MdlSkinVertexList skinVertices = parseSkinVertices(cursor, skinVertexCount);
            const MdlSkinTriangleList skinTriangles = parseSkinTriangles(cursor, skinTriangleCount);
            parseFrames(cursor, *model, frameCount, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale);

            assert(cursor <= m_end);
            return model;
        }

        void MdlParser::parseSkins(const char*& cursor, Assets::MdlModel& model, const size_t count, const size_t width, const size_t height) {
            const size_t size = width * height;
            Color avgColor;
            StringStream textureName;

            cursor = m_begin + MdlLayout::Skins;
            for (size_t i = 0; i < count; ++i) {
                const size_t skinGroup = readSize<int32_t>(cursor);
                if (skinGroup == 0) {
                    Buffer<unsigned char> rgbImage(size * 3);
                    m_palette.indexedToRgb(cursor, size, rgbImage, avgColor);
                    cursor += size;
                    
                    textureName.str();
                    textureName << m_name << "_" << i;
                    
                    Assets::Texture* texture = new Assets::Texture(textureName.str(), width, height, avgColor, rgbImage);
                    model.addSkin(new Assets::MdlSkin(texture));
                } else {
                    const size_t pictureCount = readSize<int32_t>(cursor);
                    const char* base = cursor;
                    
                    Assets::TextureList textures(pictureCount);
                    Assets::MdlTimeList times(pictureCount);
                    
                    for (size_t j = 0; j < pictureCount; ++j) {
                        cursor = base + j * sizeof(float);
                        times[j] = readFloat<float>(cursor);
                        
                        Buffer<unsigned char> rgbImage(size * 3);
                        cursor = base + pictureCount * 4 + j * size;
                        m_palette.indexedToRgb(cursor, size, rgbImage, avgColor);
                        cursor += size;

                        textureName.str();
                        textureName << m_name << "_" << i << "_" << j;

                        textures[j] = new Assets::Texture(textureName.str(), width, height, avgColor, rgbImage);
                    }
                    
                    model.addSkin(new Assets::MdlSkin(textures, times));
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
                for (size_t j = 0; j < 3; ++j)
                    triangles[i].vertices[j] = readSize<int32_t>(cursor);
            }
            return triangles;
        }

        void MdlParser::parseFrames(const char*& cursor, Assets::MdlModel& model, const size_t count, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, const size_t skinWidth, const size_t skinHeight, const Vec3f& origin, const Vec3f& scale) {
            for (size_t i = 0; i < count; ++i) {
                const int type = readInt<int32_t>(cursor);
                if (type == 0) { // single frame
                    model.addFrame(parseFrame(cursor, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale));
                } else { // frame group
                    Assets::MdlFrameGroup* frameGroup = new Assets::MdlFrameGroup();
                    
                    const char* base = cursor;
                    const size_t groupFrameCount = readSize<int32_t>(cursor);
                    
                    const char* timeCursor = base + MdlLayout::MultiFrameTimes;
                    const char* frameCursor = base + MdlLayout::MultiFrameTimes + groupFrameCount * sizeof(float);

                    for (size_t j = 0; j < groupFrameCount; ++j) {
                        const float time = readFloat<float>(timeCursor);
                        Assets::MdlFrame* frame = parseFrame(frameCursor, skinTriangles, skinVertices, skinWidth, skinHeight, origin, scale);
                        frameGroup->addFrame(frame, time);
                    }
                    
                    model.addFrame(frameGroup);
                    cursor = frameCursor;
                }
            }
        }

        Assets::MdlFrame* MdlParser::parseFrame(const char*& cursor, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, const size_t skinWidth, const size_t skinHeight, const Vec3f& origin, const Vec3f& scale) {
            char name[MdlLayout::SimpleFrameLength + 1];
            name[MdlLayout::SimpleFrameLength] = 0;
            cursor += MdlLayout::SimpleFrameName;
            readBytes(cursor, name, MdlLayout::SimpleFrameLength);
            
            PackedFrameVertexList packedVertices(skinVertices.size());
            for (size_t i = 0; i < skinVertices.size(); ++i)
                for (size_t j = 0; j < 4; ++j)
                    packedVertices[i][j] = static_cast<unsigned char>(*cursor++);
            
            Vec3f::List positions(skinVertices.size());
            BBox3f bounds;
            
            positions[0] = unpackFrameVertex(packedVertices[0], origin, scale);
            bounds.min = bounds.max = positions[0];
            
            for (size_t i = 1; i < skinVertices.size(); ++i) {
                positions[i] = unpackFrameVertex(packedVertices[i], origin, scale);
                bounds.mergeWith(positions[i]);
            }
            
            Assets::MdlFrame::VertexList frameTriangles;
            frameTriangles.reserve(skinTriangles.size());
            for (size_t i = 0; i < skinTriangles.size(); ++i) {
                const MdlSkinTriangle& triangle = skinTriangles[i];
                for (size_t j = 0; j < 3; ++j) {
                    const size_t vertexIndex = triangle.vertices[j];
                    const MdlSkinVertex& skinVertex = skinVertices[vertexIndex];
                    // const size_t normalIndex = static_cast<size_t>(packedVertices[vertexIndex].w());
                    
                    Vec2f texCoords (static_cast<float>(skinVertex.s) / static_cast<float>(skinWidth),
                                     static_cast<float>(skinVertex.t) / static_cast<float>(skinHeight));
                    if (skinVertex.onseam && !triangle.front)
                        texCoords[0] += 0.5f;
                    
                    frameTriangles.push_back(Assets::MdlFrame::Vertex(positions[vertexIndex], texCoords));
                }
            }
            
            assert(frameTriangles.size()%3 == 0);
            return new Assets::MdlFrame(String(name), frameTriangles, bounds);
        }

        Vec3f MdlParser::unpackFrameVertex(const PackedFrameVertex& vertex, const Vec3f& origin, const Vec3f& scale) const {
            Vec3f result;
            for (size_t i = 0; i < 3; ++i)
                result[i] = origin[i] + scale[i]*static_cast<float>(vertex[i]);
            return result;
        }
    }
}
