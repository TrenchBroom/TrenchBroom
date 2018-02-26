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

#include "DkmParser.h"

#include "Exceptions.h"
#include "Assets/Texture.h"
#include "Assets/Md2Model.h"
#include "Assets/Palette.h"
#include "IO/FileSystem.h"
#include "IO/ImageLoader.h"
#include "IO/IOUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace IO {
        const Vec3f DkmParser::Normals[162] = {
            Vec3f(-0.525731f, 0.000000f, 0.850651f),
            Vec3f(-0.442863f, 0.238856f, 0.864188f),
            Vec3f(-0.295242f, 0.000000f, 0.955423f),
            Vec3f(-0.309017f, 0.500000f, 0.809017f),
            Vec3f(-0.162460f, 0.262866f, 0.951056f),
            Vec3f(0.000000f, 0.000000f, 1.000000f),
            Vec3f(0.000000f, 0.850651f, 0.525731f),
            Vec3f(-0.147621f, 0.716567f, 0.681718f),
            Vec3f(0.147621f, 0.716567f, 0.681718f),
            Vec3f(0.000000f, 0.525731f, 0.850651f),
            Vec3f(0.309017f, 0.500000f, 0.809017f),
            Vec3f(0.525731f, 0.000000f, 0.850651f),
            Vec3f(0.295242f, 0.000000f, 0.955423f),
            Vec3f(0.442863f, 0.238856f, 0.864188f),
            Vec3f(0.162460f, 0.262866f, 0.951056f),
            Vec3f(-0.681718f, 0.147621f, 0.716567f),
            Vec3f(-0.809017f, 0.309017f, 0.500000f),
            Vec3f(-0.587785f, 0.425325f, 0.688191f),
            Vec3f(-0.850651f, 0.525731f, 0.000000f),
            Vec3f(-0.864188f, 0.442863f, 0.238856f),
            Vec3f(-0.716567f, 0.681718f, 0.147621f),
            Vec3f(-0.688191f, 0.587785f, 0.425325f),
            Vec3f(-0.500000f, 0.809017f, 0.309017f),
            Vec3f(-0.238856f, 0.864188f, 0.442863f),
            Vec3f(-0.425325f, 0.688191f, 0.587785f),
            Vec3f(-0.716567f, 0.681718f, -0.147621f),
            Vec3f(-0.500000f, 0.809017f, -0.309017f),
            Vec3f(-0.525731f, 0.850651f, 0.000000f),
            Vec3f(0.000000f, 0.850651f, -0.525731f),
            Vec3f(-0.238856f, 0.864188f, -0.442863f),
            Vec3f(0.000000f, 0.955423f, -0.295242f),
            Vec3f(-0.262866f, 0.951056f, -0.162460f),
            Vec3f(0.000000f, 1.000000f, 0.000000f),
            Vec3f(0.000000f, 0.955423f, 0.295242f),
            Vec3f(-0.262866f, 0.951056f, 0.162460f),
            Vec3f(0.238856f, 0.864188f, 0.442863f),
            Vec3f(0.262866f, 0.951056f, 0.162460f),
            Vec3f(0.500000f, 0.809017f, 0.309017f),
            Vec3f(0.238856f, 0.864188f, -0.442863f),
            Vec3f(0.262866f, 0.951056f, -0.162460f),
            Vec3f(0.500000f, 0.809017f, -0.309017f),
            Vec3f(0.850651f, 0.525731f, 0.000000f),
            Vec3f(0.716567f, 0.681718f, 0.147621f),
            Vec3f(0.716567f, 0.681718f, -0.147621f),
            Vec3f(0.525731f, 0.850651f, 0.000000f),
            Vec3f(0.425325f, 0.688191f, 0.587785f),
            Vec3f(0.864188f, 0.442863f, 0.238856f),
            Vec3f(0.688191f, 0.587785f, 0.425325f),
            Vec3f(0.809017f, 0.309017f, 0.500000f),
            Vec3f(0.681718f, 0.147621f, 0.716567f),
            Vec3f(0.587785f, 0.425325f, 0.688191f),
            Vec3f(0.955423f, 0.295242f, 0.000000f),
            Vec3f(1.000000f, 0.000000f, 0.000000f),
            Vec3f(0.951056f, 0.162460f, 0.262866f),
            Vec3f(0.850651f, -0.525731f, 0.000000f),
            Vec3f(0.955423f, -0.295242f, 0.000000f),
            Vec3f(0.864188f, -0.442863f, 0.238856f),
            Vec3f(0.951056f, -0.162460f, 0.262866f),
            Vec3f(0.809017f, -0.309017f, 0.500000f),
            Vec3f(0.681718f, -0.147621f, 0.716567f),
            Vec3f(0.850651f, 0.000000f, 0.525731f),
            Vec3f(0.864188f, 0.442863f, -0.238856f),
            Vec3f(0.809017f, 0.309017f, -0.500000f),
            Vec3f(0.951056f, 0.162460f, -0.262866f),
            Vec3f(0.525731f, 0.000000f, -0.850651f),
            Vec3f(0.681718f, 0.147621f, -0.716567f),
            Vec3f(0.681718f, -0.147621f, -0.716567f),
            Vec3f(0.850651f, 0.000000f, -0.525731f),
            Vec3f(0.809017f, -0.309017f, -0.500000f),
            Vec3f(0.864188f, -0.442863f, -0.238856f),
            Vec3f(0.951056f, -0.162460f, -0.262866f),
            Vec3f(0.147621f, 0.716567f, -0.681718f),
            Vec3f(0.309017f, 0.500000f, -0.809017f),
            Vec3f(0.425325f, 0.688191f, -0.587785f),
            Vec3f(0.442863f, 0.238856f, -0.864188f),
            Vec3f(0.587785f, 0.425325f, -0.688191f),
            Vec3f(0.688191f, 0.587785f, -0.425325f),
            Vec3f(-0.147621f, 0.716567f, -0.681718f),
            Vec3f(-0.309017f, 0.500000f, -0.809017f),
            Vec3f(0.000000f, 0.525731f, -0.850651f),
            Vec3f(-0.525731f, 0.000000f, -0.850651f),
            Vec3f(-0.442863f, 0.238856f, -0.864188f),
            Vec3f(-0.295242f, 0.000000f, -0.955423f),
            Vec3f(-0.162460f, 0.262866f, -0.951056f),
            Vec3f(0.000000f, 0.000000f, -1.000000f),
            Vec3f(0.295242f, 0.000000f, -0.955423f),
            Vec3f(0.162460f, 0.262866f, -0.951056f),
            Vec3f(-0.442863f, -0.238856f, -0.864188f),
            Vec3f(-0.309017f, -0.500000f, -0.809017f),
            Vec3f(-0.162460f, -0.262866f, -0.951056f),
            Vec3f(0.000000f, -0.850651f, -0.525731f),
            Vec3f(-0.147621f, -0.716567f, -0.681718f),
            Vec3f(0.147621f, -0.716567f, -0.681718f),
            Vec3f(0.000000f, -0.525731f, -0.850651f),
            Vec3f(0.309017f, -0.500000f, -0.809017f),
            Vec3f(0.442863f, -0.238856f, -0.864188f),
            Vec3f(0.162460f, -0.262866f, -0.951056f),
            Vec3f(0.238856f, -0.864188f, -0.442863f),
            Vec3f(0.500000f, -0.809017f, -0.309017f),
            Vec3f(0.425325f, -0.688191f, -0.587785f),
            Vec3f(0.716567f, -0.681718f, -0.147621f),
            Vec3f(0.688191f, -0.587785f, -0.425325f),
            Vec3f(0.587785f, -0.425325f, -0.688191f),
            Vec3f(0.000000f, -0.955423f, -0.295242f),
            Vec3f(0.000000f, -1.000000f, 0.000000f),
            Vec3f(0.262866f, -0.951056f, -0.162460f),
            Vec3f(0.000000f, -0.850651f, 0.525731f),
            Vec3f(0.000000f, -0.955423f, 0.295242f),
            Vec3f(0.238856f, -0.864188f, 0.442863f),
            Vec3f(0.262866f, -0.951056f, 0.162460f),
            Vec3f(0.500000f, -0.809017f, 0.309017f),
            Vec3f(0.716567f, -0.681718f, 0.147621f),
            Vec3f(0.525731f, -0.850651f, 0.000000f),
            Vec3f(-0.238856f, -0.864188f, -0.442863f),
            Vec3f(-0.500000f, -0.809017f, -0.309017f),
            Vec3f(-0.262866f, -0.951056f, -0.162460f),
            Vec3f(-0.850651f, -0.525731f, 0.000000f),
            Vec3f(-0.716567f, -0.681718f, -0.147621f),
            Vec3f(-0.716567f, -0.681718f, 0.147621f),
            Vec3f(-0.525731f, -0.850651f, 0.000000f),
            Vec3f(-0.500000f, -0.809017f, 0.309017f),
            Vec3f(-0.238856f, -0.864188f, 0.442863f),
            Vec3f(-0.262866f, -0.951056f, 0.162460f),
            Vec3f(-0.864188f, -0.442863f, 0.238856f),
            Vec3f(-0.809017f, -0.309017f, 0.500000f),
            Vec3f(-0.688191f, -0.587785f, 0.425325f),
            Vec3f(-0.681718f, -0.147621f, 0.716567f),
            Vec3f(-0.442863f, -0.238856f, 0.864188f),
            Vec3f(-0.587785f, -0.425325f, 0.688191f),
            Vec3f(-0.309017f, -0.500000f, 0.809017f),
            Vec3f(-0.147621f, -0.716567f, 0.681718f),
            Vec3f(-0.425325f, -0.688191f, 0.587785f),
            Vec3f(-0.162460f, -0.262866f, 0.951056f),
            Vec3f(0.442863f, -0.238856f, 0.864188f),
            Vec3f(0.162460f, -0.262866f, 0.951056f),
            Vec3f(0.309017f, -0.500000f, 0.809017f),
            Vec3f(0.147621f, -0.716567f, 0.681718f),
            Vec3f(0.000000f, -0.525731f, 0.850651f),
            Vec3f(0.425325f, -0.688191f, 0.587785f),
            Vec3f(0.587785f, -0.425325f, 0.688191f),
            Vec3f(0.688191f, -0.587785f, 0.425325f),
            Vec3f(-0.955423f, 0.295242f, 0.000000f),
            Vec3f(-0.951056f, 0.162460f, 0.262866f),
            Vec3f(-1.000000f, 0.000000f, 0.000000f),
            Vec3f(-0.850651f, 0.000000f, 0.525731f),
            Vec3f(-0.955423f, -0.295242f, 0.000000f),
            Vec3f(-0.951056f, -0.162460f, 0.262866f),
            Vec3f(-0.864188f, 0.442863f, -0.238856f),
            Vec3f(-0.951056f, 0.162460f, -0.262866f),
            Vec3f(-0.809017f, 0.309017f, -0.500000f),
            Vec3f(-0.864188f, -0.442863f, -0.238856f),
            Vec3f(-0.951056f, -0.162460f, -0.262866f),
            Vec3f(-0.809017f, -0.309017f, -0.500000f),
            Vec3f(-0.681718f, 0.147621f, -0.716567f),
            Vec3f(-0.681718f, -0.147621f, -0.716567f),
            Vec3f(-0.850651f, 0.000000f, -0.525731f),
            Vec3f(-0.688191f, 0.587785f, -0.425325f),
            Vec3f(-0.587785f, 0.425325f, -0.688191f),
            Vec3f(-0.425325f, 0.688191f, -0.587785f),
            Vec3f(-0.425325f, -0.688191f, -0.587785f),
            Vec3f(-0.587785f, -0.425325f, -0.688191f),
            Vec3f(-0.688191f, -0.587785f, -0.425325f)
        };
        
        DkmParser::DkmFrame::DkmFrame(const size_t vertexCount) :
        vertices(vertexCount) {}

        Vec3f DkmParser::DkmFrame::vertex(const size_t index) const {
            const DkmVertex& vertex = vertices[index];
            const Vec3f position(static_cast<float>(vertex.x),
                                 static_cast<float>(vertex.y),
                                 static_cast<float>(vertex.z));
            return position * scale + offset;
        }

        const Vec3f& DkmParser::DkmFrame::normal(const size_t index) const {
            const DkmVertex& vertex = vertices[index];
            return Normals[vertex.normalIndex];
        }

        DkmParser::DkmMesh::DkmMesh(const int i_vertexCount) :
        type(i_vertexCount < 0 ? Fan : Strip),
        vertexCount(static_cast<size_t>(i_vertexCount < 0 ? -i_vertexCount : i_vertexCount)),
        vertices(vertexCount) {}

        DkmParser::DkmParser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const FileSystem& fs) :
        m_name(name),
        m_begin(begin),
        /* m_end(end), */
        m_palette(palette),
        m_fs(fs) {}
        
        // http://tfc.duke.free.fr/old/models/md2.htm
        Assets::EntityModel* DkmParser::doParseModel() {
            const char* cursor = m_begin;
            const int ident = readInt<int32_t>(cursor);
            const int version = readInt<int32_t>(cursor);
            
            if (ident != DkmLayout::Ident)
                throw AssetException() << "Unknown DKM model ident: " << ident;
            if (version != DkmLayout::Version1 && version != DkmLayout::Version2)
                throw AssetException() << "Unknown DKM model version: " << version;

            const Vec3f origin = readVec3f(cursor);

            /*const size_t frameSize =*/ readSize<int32_t>(cursor);
            
            const size_t skinCount = readSize<int32_t>(cursor);
            const size_t frameVertexCount = readSize<int32_t>(cursor);
            /* const size_t texCoordCount =*/ readSize<int32_t>(cursor);
            /* const size_t triangleCount =*/ readSize<int32_t>(cursor);
            const size_t commandCount = readSize<int32_t>(cursor);
            const size_t frameCount = readSize<int32_t>(cursor);
            /* const size_t surfaceCount =*/ readSize<int32_t>(cursor);
            
            const size_t skinOffset = readSize<int32_t>(cursor);
            /* const size_t texCoordOffset =*/ readSize<int32_t>(cursor);
            /* const size_t triangleOffset =*/ readSize<int32_t>(cursor);
            const size_t frameOffset = readSize<int32_t>(cursor);
            const size_t commandOffset = readSize<int32_t>(cursor);
            /* const size_t surfaceOffset =*/ readSize<int32_t>(cursor);

            const DkmSkinList skins = parseSkins(m_begin + skinOffset, skinCount);
            const DkmFrameList frames = parseFrames(m_begin + frameOffset, frameCount, frameVertexCount, version);
            const DkmMeshList meshes = parseMeshes(m_begin + commandOffset, commandCount);
            
            return buildModel(skins, frames, meshes);
        }

        DkmParser::DkmSkinList DkmParser::parseSkins(const char* begin, const size_t skinCount) {
            DkmSkinList skins(skinCount);
            readVector(begin, skins);
            return skins;
        }

        DkmParser::DkmFrameList DkmParser::parseFrames(const char* begin, const size_t frameCount, const size_t frameVertexCount, const int version) {
            assert(version == 1 || version == 2);
            DkmFrameList frames(frameCount, DkmFrame(frameVertexCount));

            const char* cursor = begin;
            for (size_t i = 0; i < frameCount; ++i) {
                frames[i].scale = readVec3f(cursor);
                frames[i].offset = readVec3f(cursor);
                readBytes(cursor, frames[i].name, DkmLayout::FrameNameLength);

                if (version == 1) {
                    std::vector<DkmVertex1> packedVertices(frameVertexCount);
                    readVector(cursor, packedVertices);

                    for (size_t j = 0; j < frameVertexCount; ++j) {
                        frames[i].vertices[j].x = packedVertices[j].x;
                        frames[i].vertices[j].y = packedVertices[j].y;
                        frames[i].vertices[j].z = packedVertices[j].z;
                        frames[i].vertices[j].normalIndex = packedVertices[j].normalIndex;
                    }
                } else {
                    std::vector<DkmVertex2> packedVertices(frameVertexCount);
                    readVector(cursor, packedVertices);

                    /* Version 2 vertices are packed into a 32bit integer
                     * X occupies the first 11 bits
                     * Y occupies the following 11 bits
                     * Z occupies the following 10 bits
                     */
                    for (size_t j = 0; j < frameVertexCount; ++j) {
                        frames[i].vertices[j].x = (packedVertices[j].xyz & 0x7FF);
                        frames[i].vertices[j].y = (packedVertices[j].xyz & 0x1FF800) >> 11;
                        frames[i].vertices[j].z = (packedVertices[j].xyz & 0xFFE00000) >> 21;
                        frames[i].vertices[j].normalIndex = packedVertices[j].normalIndex;
                    }
                }
            }
            
            return frames;
        }

        DkmParser::DkmMeshList DkmParser::parseMeshes(const char* begin, const size_t commandCount) {
            DkmMeshList meshes;
            
            const char* cursor = begin;
            const char* end = begin + commandCount * 4;
            while (cursor < end) {
                DkmMesh mesh(readInt<int32_t>(cursor));
                for (size_t i = 0; i < mesh.vertexCount; ++i) {
                    assert(cursor < end);
                    mesh.vertices[i].texCoords[0] = readFloat<float>(cursor);
                    mesh.vertices[i].texCoords[1] = readFloat<float>(cursor);
                    mesh.vertices[i].vertexIndex = readSize<int32_t>(cursor);
                }
                meshes.push_back(mesh);
            }
            assert(cursor == end);
            
            return meshes;
        }

        Assets::EntityModel* DkmParser::buildModel(const DkmSkinList& skins, const DkmFrameList& frames, const DkmMeshList& meshes) {
            const Assets::TextureList modelTextures = loadTextures(skins);
            const Assets::Md2Model::FrameList modelFrames = buildFrames(frames, meshes);
            return new Assets::Md2Model(m_name, modelTextures, modelFrames);
        }

        Assets::TextureList DkmParser::loadTextures(const DkmSkinList& skins) {
            Assets::TextureList textures;
            textures.reserve(skins.size());
            
            try {
                for (const DkmSkin& skin : skins)
                    textures.push_back(readTexture(skin));
                return textures;
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }
        
        Assets::Texture* DkmParser::readTexture(const DkmSkin& skin) {
            const Path skinPath(String(skin.name));
            MappedFile::Ptr file = m_fs.openFile(skinPath);
            
            Color avgColor;
            const ImageLoader image(ImageLoader::PCX, file->begin(), file->end());
            
            const Buffer<unsigned char>& indices = image.indices();
            Buffer<unsigned char> rgbImage(indices.size() * 3);
            m_palette.indexedToRgb(indices, indices.size(), rgbImage, avgColor);
            
            return new Assets::Texture(skin.name, image.width(), image.height(), avgColor, rgbImage);
        }

        Assets::Md2Model::FrameList DkmParser::buildFrames(const DkmFrameList& frames, const DkmMeshList& meshes) {
            Assets::Md2Model::FrameList modelFrames;
            modelFrames.reserve(frames.size());
            
            for (const DkmFrame& frame : frames)
                modelFrames.push_back(buildFrame(frame, meshes));
            return modelFrames;
        }

        Assets::Md2Model::Frame* DkmParser::buildFrame(const DkmFrame& frame, const DkmMeshList& meshes) {
            size_t vertexCount = 0;
            Renderer::IndexRangeMap::Size size;
            for (const DkmMesh& md2Mesh : meshes) {
                vertexCount += md2Mesh.vertices.size();
                if (md2Mesh.type == DkmMesh::Fan)
                    size.inc(GL_TRIANGLE_FAN);
                else
                    size.inc(GL_TRIANGLE_STRIP);
            }

            Renderer::IndexRangeMapBuilder<Assets::Md2Model::VertexSpec> builder(vertexCount, size);
            for (const DkmMesh& md2Mesh : meshes) {
                if (!md2Mesh.vertices.empty()) {
                    vertexCount += md2Mesh.vertices.size();
                    if (md2Mesh.type == DkmMesh::Fan)
                        builder.addTriangleFan(getVertices(frame, md2Mesh.vertices));
                    else
                        builder.addTriangleStrip(getVertices(frame, md2Mesh.vertices));
                }
            }
            
            return new Assets::Md2Model::Frame(builder.vertices(), builder.indexArray());
        }
        
        Assets::Md2Model::VertexList DkmParser::getVertices(const DkmFrame& frame, const DkmMeshVertexList& meshVertices) const {
            typedef Assets::Md2Model::Vertex Vertex;

            Vertex::List result(0);
            result.reserve(meshVertices.size());
            
            for (const DkmMeshVertex& md2MeshVertex : meshVertices) {
                const Vec3f position = frame.vertex(md2MeshVertex.vertexIndex);
                const Vec3f& normal = frame.normal(md2MeshVertex.vertexIndex);
                const Vec2f& texCoords = md2MeshVertex.texCoords;
                
                result.push_back(Vertex(position, normal, texCoords));
            }
            
            return result;
        }
    }
}
