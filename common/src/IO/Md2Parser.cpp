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

#include "Md2Parser.h"

#include "Exceptions.h"
#include "Assets/Texture.h"
#include "Assets/Md2Model.h"
#include "Assets/Palette.h"
#include "IO/GameFileSystem.h"
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
        const Vec3f Md2Parser::Normals[162] = {
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
        
        Md2Parser::Md2Frame::Md2Frame(const size_t vertexCount) :
        vertices(vertexCount) {}

        Vec3f Md2Parser::Md2Frame::vertex(const size_t index) const {
            const Md2Vertex& vertex = vertices[index];
            const Vec3f position(static_cast<float>(vertex.x),
                                 static_cast<float>(vertex.y),
                                 static_cast<float>(vertex.z));
            return position * scale + offset;
        }

        const Vec3f& Md2Parser::Md2Frame::normal(const size_t index) const {
            const Md2Vertex& vertex = vertices[index];
            return Normals[vertex.normalIndex];
        }

        Md2Parser::Md2Mesh::Md2Mesh(const int i_vertexCount) :
        type(i_vertexCount < 0 ? Fan : Strip),
        vertexCount(static_cast<size_t>(i_vertexCount < 0 ? -i_vertexCount : i_vertexCount)),
        vertices(vertexCount) {}

        Md2Parser::Md2Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const GameFileSystem& fs) :
        m_name(name),
        m_begin(begin),
        /* m_end(end), */
        m_palette(palette),
        m_fs(fs) {}
        
        // http://tfc.duke.free.fr/old/models/md2.htm
        Assets::EntityModel* Md2Parser::doParseModel() {
            const char* cursor = m_begin;
            const int ident = readInt<int32_t>(cursor);
            const int version = readInt<int32_t>(cursor);
            
            if (ident != Md2Layout::Ident)
                throw AssetException() << "Unknown MD2 model ident: " << ident;
            if (version != Md2Layout::Version)
                throw AssetException() << "Unknown MD2 model version: " << version;
            
            /*const size_t skinWidth =*/ readSize<int32_t>(cursor);
            /*const size_t skinHeight =*/ readSize<int32_t>(cursor);
            /*const size_t frameSize =*/ readSize<int32_t>(cursor);
            
            const size_t skinCount = readSize<int32_t>(cursor);
            const size_t frameVertexCount = readSize<int32_t>(cursor);
            /* const size_t texCoordCount =*/ readSize<int32_t>(cursor);
            /* const size_t triangleCount =*/ readSize<int32_t>(cursor);
            const size_t commandCount = readSize<int32_t>(cursor);
            const size_t frameCount = readSize<int32_t>(cursor);
            
            const size_t skinOffset = readSize<int32_t>(cursor);
            /* const size_t texCoordOffset =*/ readSize<int32_t>(cursor);
            /* const size_t triangleOffset =*/ readSize<int32_t>(cursor);
            const size_t frameOffset = readSize<int32_t>(cursor);
            const size_t commandOffset = readSize<int32_t>(cursor);

            const Md2SkinList skins = parseSkins(m_begin + skinOffset, skinCount);
            const Md2FrameList frames = parseFrames(m_begin + frameOffset, frameCount, frameVertexCount);
            const Md2MeshList meshes = parseMeshes(m_begin + commandOffset, commandCount);
            
            return buildModel(skins, frames, meshes);
        }

        Md2Parser::Md2SkinList Md2Parser::parseSkins(const char* begin, const size_t skinCount) {
            Md2SkinList skins(skinCount);
            readVector(begin, skins);
            return skins;
        }

        Md2Parser::Md2FrameList Md2Parser::parseFrames(const char* begin, const size_t frameCount, const size_t frameVertexCount) {
            Md2FrameList frames(frameCount, Md2Frame(frameVertexCount));

            const char* cursor = begin;
            for (size_t i = 0; i < frameCount; ++i) {
                frames[i].scale = readVec3f(cursor);
                frames[i].offset = readVec3f(cursor);
                readBytes(cursor, frames[i].name, Md2Layout::FrameNameLength);
                readVector(cursor, frames[i].vertices);
            }
            
            return frames;
        }

        Md2Parser::Md2MeshList Md2Parser::parseMeshes(const char* begin, const size_t commandCount) {
            Md2MeshList meshes;
            
            const char* cursor = begin;
            const char* end = begin + commandCount * 4;
            while (cursor < end) {
                Md2Mesh mesh(readInt<int32_t>(cursor));
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

        Assets::EntityModel* Md2Parser::buildModel(const Md2SkinList& skins, const Md2FrameList& frames, const Md2MeshList& meshes) {
            const Assets::TextureList modelTextures = loadTextures(skins);
            const Assets::Md2Model::FrameList modelFrames = buildFrames(frames, meshes);
            return new Assets::Md2Model(m_name, modelTextures, modelFrames);
        }

        Assets::TextureList Md2Parser::loadTextures(const Md2SkinList& skins) {
            Assets::TextureList textures;
            textures.reserve(skins.size());
            
            try {
                Md2SkinList::const_iterator it, end;
                for (it = skins.begin(), end = skins.end(); it != end; ++it) {
                    const Md2Skin& skin = *it;
                    Assets::Texture* texture = loadTexture(skin);
                    textures.push_back(texture);
                }
                return textures;
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }
        
        Assets::Texture* Md2Parser::loadTexture(const Md2Skin& skin) {
            const Path skinPath(String(skin.name));
            MappedFile::Ptr file = m_fs.openFile(skinPath);
            
            Color avgColor;
            const ImageLoader image(ImageLoader::PCX, file->begin(), file->end());
            
            const Buffer<unsigned char>& indices = image.indices();
            Buffer<unsigned char> rgbImage(indices.size() * 3);
            m_palette.indexedToRgb(indices, indices.size(), rgbImage, avgColor);
            
            return new Assets::Texture(skin.name, image.width(), image.height(), avgColor, rgbImage);
        }

        Assets::Md2Model::FrameList Md2Parser::buildFrames(const Md2FrameList& frames, const Md2MeshList& meshes) {
            Assets::Md2Model::FrameList modelFrames;
            modelFrames.reserve(frames.size());
            
            Md2FrameList::const_iterator it, end;
            for (it = frames.begin(), end = frames.end(); it != end; ++it) {
                const Md2Frame& frame = *it;
                Assets::Md2Model::Frame* modelFrame = buildFrame(frame, meshes);
                modelFrames.push_back(modelFrame);
            }
            return modelFrames;
        }

        Assets::Md2Model::Frame* Md2Parser::buildFrame(const Md2Frame& frame, const Md2MeshList& meshes) {
            Md2MeshList::const_iterator mIt, mEnd;

            size_t vertexCount = 0;
            Renderer::IndexRangeMap::Size size;
            for (mIt = meshes.begin(), mEnd = meshes.end(); mIt != mEnd; ++mIt) {
                const Md2Mesh& md2Mesh = *mIt;
                vertexCount += md2Mesh.vertices.size();
                if (md2Mesh.type == Md2Mesh::Fan)
                    size.inc(GL_TRIANGLE_FAN);
                else
                    size.inc(GL_TRIANGLE_STRIP);
            }

            Renderer::IndexRangeMapBuilder<Assets::Md2Model::VertexSpec> builder(vertexCount, size);
            for (mIt = meshes.begin(), mEnd = meshes.end(); mIt != mEnd; ++mIt) {
                const Md2Mesh& md2Mesh = *mIt;
                if (md2Mesh.vertices.size() > 0) {
                    vertexCount += md2Mesh.vertices.size();
                    if (md2Mesh.type == Md2Mesh::Fan)
                        builder.addTriangleFan(getVertices(frame, md2Mesh.vertices));
                    else
                        builder.addTriangleStrip(getVertices(frame, md2Mesh.vertices));
                }
            }
            
            return new Assets::Md2Model::Frame(builder.vertices(), builder.indexArray());
        }
        
        Assets::Md2Model::VertexList Md2Parser::getVertices(const Md2Frame& frame, const Md2MeshVertexList& meshVertices) const {
            typedef Assets::Md2Model::Vertex Vertex;

            Vertex::List result(0);
            result.reserve(meshVertices.size());
            
            Md2MeshVertexList::const_iterator it, end;
            for (it = meshVertices.begin(), end = meshVertices.end(); it != end; ++it) {
                const Md2MeshVertex& md2MeshVertex = *it;
                
                const Vec3f position = frame.vertex(md2MeshVertex.vertexIndex);
                const Vec3f& normal = frame.normal(md2MeshVertex.vertexIndex);
                const Vec2f& texCoords = md2MeshVertex.texCoords;
                
                result.push_back(Vertex(position, normal, texCoords));
            }
            
            return result;
        }
    }
}
