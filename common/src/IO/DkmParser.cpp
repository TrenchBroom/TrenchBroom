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
#include "Assets/Palette.h"
#include "IO/FileSystem.h"
#include "IO/ImageLoader.h"
#include "IO/IOUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"
#include "IO/SkinLoader.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace IO {
        const vm::vec3f DkmParser::Normals[162] = {
            vm::vec3f(-0.525731f, 0.000000f, 0.850651f),
            vm::vec3f(-0.442863f, 0.238856f, 0.864188f),
            vm::vec3f(-0.295242f, 0.000000f, 0.955423f),
            vm::vec3f(-0.309017f, 0.500000f, 0.809017f),
            vm::vec3f(-0.162460f, 0.262866f, 0.951056f),
            vm::vec3f(0.000000f, 0.000000f, 1.000000f),
            vm::vec3f(0.000000f, 0.850651f, 0.525731f),
            vm::vec3f(-0.147621f, 0.716567f, 0.681718f),
            vm::vec3f(0.147621f, 0.716567f, 0.681718f),
            vm::vec3f(0.000000f, 0.525731f, 0.850651f),
            vm::vec3f(0.309017f, 0.500000f, 0.809017f),
            vm::vec3f(0.525731f, 0.000000f, 0.850651f),
            vm::vec3f(0.295242f, 0.000000f, 0.955423f),
            vm::vec3f(0.442863f, 0.238856f, 0.864188f),
            vm::vec3f(0.162460f, 0.262866f, 0.951056f),
            vm::vec3f(-0.681718f, 0.147621f, 0.716567f),
            vm::vec3f(-0.809017f, 0.309017f, 0.500000f),
            vm::vec3f(-0.587785f, 0.425325f, 0.688191f),
            vm::vec3f(-0.850651f, 0.525731f, 0.000000f),
            vm::vec3f(-0.864188f, 0.442863f, 0.238856f),
            vm::vec3f(-0.716567f, 0.681718f, 0.147621f),
            vm::vec3f(-0.688191f, 0.587785f, 0.425325f),
            vm::vec3f(-0.500000f, 0.809017f, 0.309017f),
            vm::vec3f(-0.238856f, 0.864188f, 0.442863f),
            vm::vec3f(-0.425325f, 0.688191f, 0.587785f),
            vm::vec3f(-0.716567f, 0.681718f, -0.147621f),
            vm::vec3f(-0.500000f, 0.809017f, -0.309017f),
            vm::vec3f(-0.525731f, 0.850651f, 0.000000f),
            vm::vec3f(0.000000f, 0.850651f, -0.525731f),
            vm::vec3f(-0.238856f, 0.864188f, -0.442863f),
            vm::vec3f(0.000000f, 0.955423f, -0.295242f),
            vm::vec3f(-0.262866f, 0.951056f, -0.162460f),
            vm::vec3f(0.000000f, 1.000000f, 0.000000f),
            vm::vec3f(0.000000f, 0.955423f, 0.295242f),
            vm::vec3f(-0.262866f, 0.951056f, 0.162460f),
            vm::vec3f(0.238856f, 0.864188f, 0.442863f),
            vm::vec3f(0.262866f, 0.951056f, 0.162460f),
            vm::vec3f(0.500000f, 0.809017f, 0.309017f),
            vm::vec3f(0.238856f, 0.864188f, -0.442863f),
            vm::vec3f(0.262866f, 0.951056f, -0.162460f),
            vm::vec3f(0.500000f, 0.809017f, -0.309017f),
            vm::vec3f(0.850651f, 0.525731f, 0.000000f),
            vm::vec3f(0.716567f, 0.681718f, 0.147621f),
            vm::vec3f(0.716567f, 0.681718f, -0.147621f),
            vm::vec3f(0.525731f, 0.850651f, 0.000000f),
            vm::vec3f(0.425325f, 0.688191f, 0.587785f),
            vm::vec3f(0.864188f, 0.442863f, 0.238856f),
            vm::vec3f(0.688191f, 0.587785f, 0.425325f),
            vm::vec3f(0.809017f, 0.309017f, 0.500000f),
            vm::vec3f(0.681718f, 0.147621f, 0.716567f),
            vm::vec3f(0.587785f, 0.425325f, 0.688191f),
            vm::vec3f(0.955423f, 0.295242f, 0.000000f),
            vm::vec3f(1.000000f, 0.000000f, 0.000000f),
            vm::vec3f(0.951056f, 0.162460f, 0.262866f),
            vm::vec3f(0.850651f, -0.525731f, 0.000000f),
            vm::vec3f(0.955423f, -0.295242f, 0.000000f),
            vm::vec3f(0.864188f, -0.442863f, 0.238856f),
            vm::vec3f(0.951056f, -0.162460f, 0.262866f),
            vm::vec3f(0.809017f, -0.309017f, 0.500000f),
            vm::vec3f(0.681718f, -0.147621f, 0.716567f),
            vm::vec3f(0.850651f, 0.000000f, 0.525731f),
            vm::vec3f(0.864188f, 0.442863f, -0.238856f),
            vm::vec3f(0.809017f, 0.309017f, -0.500000f),
            vm::vec3f(0.951056f, 0.162460f, -0.262866f),
            vm::vec3f(0.525731f, 0.000000f, -0.850651f),
            vm::vec3f(0.681718f, 0.147621f, -0.716567f),
            vm::vec3f(0.681718f, -0.147621f, -0.716567f),
            vm::vec3f(0.850651f, 0.000000f, -0.525731f),
            vm::vec3f(0.809017f, -0.309017f, -0.500000f),
            vm::vec3f(0.864188f, -0.442863f, -0.238856f),
            vm::vec3f(0.951056f, -0.162460f, -0.262866f),
            vm::vec3f(0.147621f, 0.716567f, -0.681718f),
            vm::vec3f(0.309017f, 0.500000f, -0.809017f),
            vm::vec3f(0.425325f, 0.688191f, -0.587785f),
            vm::vec3f(0.442863f, 0.238856f, -0.864188f),
            vm::vec3f(0.587785f, 0.425325f, -0.688191f),
            vm::vec3f(0.688191f, 0.587785f, -0.425325f),
            vm::vec3f(-0.147621f, 0.716567f, -0.681718f),
            vm::vec3f(-0.309017f, 0.500000f, -0.809017f),
            vm::vec3f(0.000000f, 0.525731f, -0.850651f),
            vm::vec3f(-0.525731f, 0.000000f, -0.850651f),
            vm::vec3f(-0.442863f, 0.238856f, -0.864188f),
            vm::vec3f(-0.295242f, 0.000000f, -0.955423f),
            vm::vec3f(-0.162460f, 0.262866f, -0.951056f),
            vm::vec3f(0.000000f, 0.000000f, -1.000000f),
            vm::vec3f(0.295242f, 0.000000f, -0.955423f),
            vm::vec3f(0.162460f, 0.262866f, -0.951056f),
            vm::vec3f(-0.442863f, -0.238856f, -0.864188f),
            vm::vec3f(-0.309017f, -0.500000f, -0.809017f),
            vm::vec3f(-0.162460f, -0.262866f, -0.951056f),
            vm::vec3f(0.000000f, -0.850651f, -0.525731f),
            vm::vec3f(-0.147621f, -0.716567f, -0.681718f),
            vm::vec3f(0.147621f, -0.716567f, -0.681718f),
            vm::vec3f(0.000000f, -0.525731f, -0.850651f),
            vm::vec3f(0.309017f, -0.500000f, -0.809017f),
            vm::vec3f(0.442863f, -0.238856f, -0.864188f),
            vm::vec3f(0.162460f, -0.262866f, -0.951056f),
            vm::vec3f(0.238856f, -0.864188f, -0.442863f),
            vm::vec3f(0.500000f, -0.809017f, -0.309017f),
            vm::vec3f(0.425325f, -0.688191f, -0.587785f),
            vm::vec3f(0.716567f, -0.681718f, -0.147621f),
            vm::vec3f(0.688191f, -0.587785f, -0.425325f),
            vm::vec3f(0.587785f, -0.425325f, -0.688191f),
            vm::vec3f(0.000000f, -0.955423f, -0.295242f),
            vm::vec3f(0.000000f, -1.000000f, 0.000000f),
            vm::vec3f(0.262866f, -0.951056f, -0.162460f),
            vm::vec3f(0.000000f, -0.850651f, 0.525731f),
            vm::vec3f(0.000000f, -0.955423f, 0.295242f),
            vm::vec3f(0.238856f, -0.864188f, 0.442863f),
            vm::vec3f(0.262866f, -0.951056f, 0.162460f),
            vm::vec3f(0.500000f, -0.809017f, 0.309017f),
            vm::vec3f(0.716567f, -0.681718f, 0.147621f),
            vm::vec3f(0.525731f, -0.850651f, 0.000000f),
            vm::vec3f(-0.238856f, -0.864188f, -0.442863f),
            vm::vec3f(-0.500000f, -0.809017f, -0.309017f),
            vm::vec3f(-0.262866f, -0.951056f, -0.162460f),
            vm::vec3f(-0.850651f, -0.525731f, 0.000000f),
            vm::vec3f(-0.716567f, -0.681718f, -0.147621f),
            vm::vec3f(-0.716567f, -0.681718f, 0.147621f),
            vm::vec3f(-0.525731f, -0.850651f, 0.000000f),
            vm::vec3f(-0.500000f, -0.809017f, 0.309017f),
            vm::vec3f(-0.238856f, -0.864188f, 0.442863f),
            vm::vec3f(-0.262866f, -0.951056f, 0.162460f),
            vm::vec3f(-0.864188f, -0.442863f, 0.238856f),
            vm::vec3f(-0.809017f, -0.309017f, 0.500000f),
            vm::vec3f(-0.688191f, -0.587785f, 0.425325f),
            vm::vec3f(-0.681718f, -0.147621f, 0.716567f),
            vm::vec3f(-0.442863f, -0.238856f, 0.864188f),
            vm::vec3f(-0.587785f, -0.425325f, 0.688191f),
            vm::vec3f(-0.309017f, -0.500000f, 0.809017f),
            vm::vec3f(-0.147621f, -0.716567f, 0.681718f),
            vm::vec3f(-0.425325f, -0.688191f, 0.587785f),
            vm::vec3f(-0.162460f, -0.262866f, 0.951056f),
            vm::vec3f(0.442863f, -0.238856f, 0.864188f),
            vm::vec3f(0.162460f, -0.262866f, 0.951056f),
            vm::vec3f(0.309017f, -0.500000f, 0.809017f),
            vm::vec3f(0.147621f, -0.716567f, 0.681718f),
            vm::vec3f(0.000000f, -0.525731f, 0.850651f),
            vm::vec3f(0.425325f, -0.688191f, 0.587785f),
            vm::vec3f(0.587785f, -0.425325f, 0.688191f),
            vm::vec3f(0.688191f, -0.587785f, 0.425325f),
            vm::vec3f(-0.955423f, 0.295242f, 0.000000f),
            vm::vec3f(-0.951056f, 0.162460f, 0.262866f),
            vm::vec3f(-1.000000f, 0.000000f, 0.000000f),
            vm::vec3f(-0.850651f, 0.000000f, 0.525731f),
            vm::vec3f(-0.955423f, -0.295242f, 0.000000f),
            vm::vec3f(-0.951056f, -0.162460f, 0.262866f),
            vm::vec3f(-0.864188f, 0.442863f, -0.238856f),
            vm::vec3f(-0.951056f, 0.162460f, -0.262866f),
            vm::vec3f(-0.809017f, 0.309017f, -0.500000f),
            vm::vec3f(-0.864188f, -0.442863f, -0.238856f),
            vm::vec3f(-0.951056f, -0.162460f, -0.262866f),
            vm::vec3f(-0.809017f, -0.309017f, -0.500000f),
            vm::vec3f(-0.681718f, 0.147621f, -0.716567f),
            vm::vec3f(-0.681718f, -0.147621f, -0.716567f),
            vm::vec3f(-0.850651f, 0.000000f, -0.525731f),
            vm::vec3f(-0.688191f, 0.587785f, -0.425325f),
            vm::vec3f(-0.587785f, 0.425325f, -0.688191f),
            vm::vec3f(-0.425325f, 0.688191f, -0.587785f),
            vm::vec3f(-0.425325f, -0.688191f, -0.587785f),
            vm::vec3f(-0.587785f, -0.425325f, -0.688191f),
            vm::vec3f(-0.688191f, -0.587785f, -0.425325f)
        };
        
        DkmParser::DkmFrame::DkmFrame(const size_t vertexCount) :
        name(""),
        vertices(vertexCount) {}

        vm::vec3f DkmParser::DkmFrame::vertex(const size_t index) const {
            assert(index < vertices.size());
            
            const DkmVertex& vertex = vertices[index];
            const vm::vec3f position(static_cast<float>(vertex.x),
                                 static_cast<float>(vertex.y),
                                 static_cast<float>(vertex.z));
            return position * scale + offset;
        }

        const vm::vec3f& DkmParser::DkmFrame::normal(const size_t index) const {
            assert(index < vertices.size());
            
            const DkmVertex& vertex = vertices[index];
            return Normals[vertex.normalIndex];
        }

        DkmParser::DkmMesh::DkmMesh(const int i_vertexCount) :
        type(i_vertexCount < 0 ? Fan : Strip),
        vertexCount(static_cast<size_t>(i_vertexCount < 0 ? -i_vertexCount : i_vertexCount)),
        vertices(vertexCount) {}

        DkmParser::DkmParser(const String& name, const char* begin, const char* end, const FileSystem& fs) :
        m_name(name),
        m_begin(begin),
        /* m_end(end), */
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

            /* const vm::vec3f origin = */ readVec3f(cursor);

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
                     * Y occupies the following 10 bits
                     * Z occupies the following 11 bits
                     */
                    for (size_t j = 0; j < frameVertexCount; ++j) {
                        frames[i].vertices[j].x = (packedVertices[j].xyz & 0xFFE00000) >> 21;
                        frames[i].vertices[j].y = (packedVertices[j].xyz & 0x1FF800) >> 11;
                        frames[i].vertices[j].z = (packedVertices[j].xyz & 0x7FF);
                        frames[i].vertices[j].normalIndex = packedVertices[j].normalIndex;
                    }
                }
            }
            
            return frames;
        }

        DkmParser::DkmMeshList DkmParser::parseMeshes(const char* begin, const size_t commandCount) {
            DkmMeshList meshes;
            
            const char* cursor = begin;
            auto vertexCount = readInt<int32_t>(cursor);
            while (vertexCount != 0) {
                /* const int skinIndex    = */ readInt<int32_t>(cursor);
                /* const int surfaceIndex = */ readInt<int32_t>(cursor);
                
                DkmMesh mesh(vertexCount);
                for (size_t i = 0; i < mesh.vertexCount; ++i) {
                    mesh.vertices[i].vertexIndex = readSize<int32_t>(cursor); // index before texcoords in DKM

                    mesh.vertices[i].texCoords[0] = readFloat<float>(cursor);
                    mesh.vertices[i].texCoords[1] = readFloat<float>(cursor);
                }
                meshes.push_back(mesh);
                vertexCount = readInt<int32_t>(cursor);
            }
        
            return meshes;
        }

        Assets::EntityModel* DkmParser::buildModel(const DkmSkinList& skins, const DkmFrameList& frames, const DkmMeshList& meshes) {
            using ModelPtr = std::unique_ptr<Assets::EntityModel>;
            ModelPtr model = std::make_unique<Assets::EntityModel>(m_name);

            loadSkins(model.get(), skins);
            buildFrames(model.get(), frames, meshes);

            return model.release();
        }

        void DkmParser::loadSkins(Assets::EntityModel* model, const DkmParser::DkmSkinList& skins) {
            for (const auto& skin : skins) {
                const auto skinPath = findSkin(skin);
                auto* texture = loadSkin(m_fs.openFile(skinPath));
                if (texture != nullptr) {
                    model->addSkin(texture);
                }
            }
        }

        /**
         * Daikatana's models contain wrong skin paths. They often refer to a skin like "x/y.bmp" which does
         * not exist, and the correct skin file name will be "x/y.wal" instead. That's why we try to find
         * a matching file name by disregarding the extension.
         */
        const IO::Path DkmParser::findSkin(const DkmSkin& skin) const {
            const Path skinPath(String(skin.name));
            if (m_fs.fileExists(skinPath)) {
                return skinPath;
            }

            // try "wal" extension instead
            if (StringUtils::toLower(skinPath.extension()) == "bmp") {
                const auto walPath = skinPath.replaceExtension("wal");
                if (m_fs.fileExists(walPath)) {
                    return walPath;
                }
            }

            // Search for any file with the correct base name.
            const auto folder = skinPath.deleteLastComponent();
            const auto basename = skinPath.lastComponent().deleteExtension();
            const auto items = m_fs.findItems(folder, FileNameMatcher(basename.addExtension("*").asString()));
            if (items.size() == 1) {
                return items.front();
            } else {
                return skinPath;
            }
        }

        void DkmParser::buildFrames(Assets::EntityModel* model, const DkmParser::DkmFrameList& frames, const DkmParser::DkmMeshList& meshes) {
            for (const auto& frame: frames) {
                size_t vertexCount = 0;
                Renderer::IndexRangeMap::Size size;
                for (const auto& md2Mesh : meshes) {
                    vertexCount += md2Mesh.vertices.size();
                    if (md2Mesh.type == DkmMesh::Fan)
                        size.inc(GL_TRIANGLE_FAN);
                    else
                        size.inc(GL_TRIANGLE_STRIP);
                }

                Renderer::IndexRangeMapBuilder<Assets::EntityModel::Vertex::Spec> builder(vertexCount, size);
                for (const auto& md2Mesh : meshes) {
                    if (!md2Mesh.vertices.empty()) {
                        vertexCount += md2Mesh.vertices.size();
                        if (md2Mesh.type == DkmMesh::Fan)
                            builder.addTriangleFan(getVertices(frame, md2Mesh.vertices));
                        else
                            builder.addTriangleStrip(getVertices(frame, md2Mesh.vertices));
                    }
                }

                model->addFrame(frame.name, builder.vertices(), builder.indexArray());
            }
        }

        Assets::EntityModel::VertexList DkmParser::getVertices(const DkmFrame& frame, const DkmMeshVertexList& meshVertices) const {
            typedef Assets::EntityModel::Vertex Vertex;

            Vertex::List result(0);
            result.reserve(meshVertices.size());
            
            for (const DkmMeshVertex& md2MeshVertex : meshVertices) {
                const auto position = frame.vertex(md2MeshVertex.vertexIndex);
                const auto& texCoords = md2MeshVertex.texCoords;
                
                result.push_back(Vertex(position, texCoords));
            }
            
            return result;
        }
    }
}
