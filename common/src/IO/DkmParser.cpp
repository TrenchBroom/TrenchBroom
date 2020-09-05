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
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/FileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/SkinLoader.h"
#include "Renderer/GLVertex.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include <kdl/string_format.h>

#include <string>

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

        DkmParser::DkmParser(const std::string& name, const char* begin, const char* end, const FileSystem& fs) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_fs(fs) {}

        // http://tfc.duke.free.fr/old/models/md2.htm
        std::unique_ptr<Assets::EntityModel> DkmParser::doInitializeModel(Logger& logger) {
            auto reader = Reader::from(m_begin, m_end);

            const int ident = reader.readInt<int32_t>();
            const int version = reader.readInt<int32_t>();

            if (ident != DkmLayout::Ident) {
                throw AssetException("Unknown DKM model ident: " + std::to_string(ident));
            }
            if (version != DkmLayout::Version1 && version != DkmLayout::Version2) {
                throw AssetException("Unknown DKM model version: " + std::to_string(version));
            }

            /* const auto origin = */ reader.readVec<float,3>();

            /* const auto frameSize = */ reader.readSize<int32_t>();

            const auto skinCount = reader.readSize<int32_t>();
            /* const auto vertexCount = */ reader.readSize<int32_t>();
            /* const auto texCoordCount =*/ reader.readSize<int32_t>();
            /* const auto triangleCount =*/ reader.readSize<int32_t>();
            /* const auto commandCount = */ reader.readSize<int32_t>();
            const auto frameCount = reader.readSize<int32_t>();
            /* const auto surfaceCount =*/ reader.readSize<int32_t>();

            const auto skinOffset = reader.readSize<int32_t>();

            const auto skins = parseSkins(reader.subReaderFromBegin(skinOffset), skinCount);

            auto model = std::make_unique<Assets::EntityModel>(m_name, Assets::PitchType::Normal);
            model->addFrames(frameCount);

            auto& surface = model->addSurface(m_name);
            loadSkins(surface, skins, logger);

            return model;
        }

        void DkmParser::doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */) {
            auto reader = Reader::from(m_begin, m_end);
            const int ident = reader.readInt<int32_t>();
            const int version = reader.readInt<int32_t>();

            if (ident != DkmLayout::Ident) {
                throw AssetException("Unknown DKM model ident: " + std::to_string(ident));
            }
            if (version != DkmLayout::Version1 && version != DkmLayout::Version2) {
                throw AssetException("Unknown DKM model version: " + std::to_string(version));
            }

            /* const auto origin = */ reader.readVec<float,3>();

            const auto frameSize = reader.readSize<int32_t>();

            /* const auto skinCount = */ reader.readSize<int32_t>();
            const auto vertexCount = reader.readSize<int32_t>();
            /* const auto texCoordCount =*/ reader.readSize<int32_t>();
            /* const auto triangleCount =*/ reader.readSize<int32_t>();
            const auto commandCount = reader.readSize<int32_t>();
            /* const auto frameCount = */ reader.readSize<int32_t>();
            /* const auto surfaceCount =*/ reader.readSize<int32_t>();

            /* const auto skinOffset = */ reader.readSize<int32_t>();
            /* const auto texCoordOffset =*/ reader.readSize<int32_t>();
            /* const auto triangleOffset =*/ reader.readSize<int32_t>();
            const auto frameOffset = reader.readSize<int32_t>();
            const auto commandOffset = reader.readSize<int32_t>();
            /* const auto surfaceOffset =*/ reader.readSize<int32_t>();

            const auto frame = parseFrame(reader.subReaderFromBegin(frameOffset + frameIndex * frameSize, frameSize), frameIndex, vertexCount, version);
            const auto meshes = parseMeshes(reader.subReaderFromBegin(commandOffset, commandCount * 4), commandCount);

            auto& surface = model.surface(0);
            buildFrame(model, surface, frameIndex, frame, meshes);
        }

        DkmParser::DkmSkinList DkmParser::parseSkins(Reader reader, const size_t skinCount) {
            DkmSkinList skins;
            skins.reserve(skinCount);
            for (size_t i = 0; i < skinCount; ++i) {
                skins.emplace_back(reader.readString(DkmLayout::SkinNameLength));
            }
            return skins;
        }

        DkmParser::DkmFrame DkmParser::parseFrame(Reader reader, const size_t /* frameIndex */, const size_t vertexCount, const int version) {
            assert(version == 1 || version == 2);

            auto frame = DkmFrame(vertexCount);

            frame.scale = reader.readVec<float,3>();
            frame.offset = reader.readVec<float,3>();
            frame.name = reader.readString(DkmLayout::FrameNameLength);

            assert(!vm::is_nan(frame.scale));
            assert(!vm::is_nan(frame.offset));

            if (version == 1) {
                for (size_t i = 0; i < vertexCount; ++i) {
                    frame.vertices[i].x = reader.readUnsignedChar<char>();
                    frame.vertices[i].y = reader.readUnsignedChar<char>();
                    frame.vertices[i].z = reader.readUnsignedChar<char>();
                    frame.vertices[i].normalIndex = reader.readUnsignedChar<char>();
                }
            } else {
                /* Version 2 vertices are packed into a 32bit integer
                 * X occupies the first 11 bits
                 * Y occupies the following 10 bits
                 * Z occupies the following 11 bits
                 */
                for (size_t i = 0; i < vertexCount; ++i) {
                    const auto packedPosition = reader.read<uint32_t, uint32_t>();
                    frame.vertices[i].x = (packedPosition & 0xFFE00000) >> 21;
                    frame.vertices[i].y = (packedPosition & 0x1FF800) >> 11;
                    frame.vertices[i].z = (packedPosition & 0x7FF);
                    frame.vertices[i].normalIndex = reader.readUnsignedChar<char>();
                }
            }

            return frame;
        }

        DkmParser::DkmMeshList DkmParser::parseMeshes(Reader reader, const size_t /* commandCount */) {
            DkmMeshList meshes;

            // vertex count is signed, where < 0 indicates a triangle fan and > 0 indicates a triangle strip
            auto vertexCount = reader.readInt<int32_t>();
            while (vertexCount != 0) {
                /* const size_t skinIndex    = */ reader.readSize<int32_t>();
                /* const size_t surfaceIndex = */ reader.readSize<int32_t>();

                DkmMesh mesh(vertexCount);
                for (size_t i = 0; i < mesh.vertexCount; ++i) {
                    mesh.vertices[i].vertexIndex = reader.readSize<int32_t>(); // index before texcoords in DKM
                    mesh.vertices[i].texCoords[0] = reader.readFloat<float>();
                    mesh.vertices[i].texCoords[1] = reader.readFloat<float>();
                }
                meshes.push_back(mesh);
                vertexCount = reader.readInt<int32_t>();
            }

            return meshes;
        }

        void DkmParser::loadSkins(Assets::EntityModelSurface& surface, const DkmParser::DkmSkinList& skins, Logger& logger) {
            std::vector<Assets::Texture> textures;
            textures.reserve(skins.size());
            
            for (const auto& skin : skins) {
                const auto skinPath = findSkin(skin);
                textures.push_back(loadSkin(skinPath, m_fs, logger));
            }
            
            surface.setSkins(std::move(textures));
        }

        /**
         * Daikatana's models contain wrong skin paths. They often refer to a skin like "x/y.bmp" which does
         * not exist, and the correct skin file name will be "x/y.wal" instead. That's why we try to find
         * a matching file name by disregarding the extension.
         */
        Path DkmParser::findSkin(const std::string& skin) const {
            const Path skinPath(skin);
            if (m_fs.fileExists(skinPath)) {
                return skinPath;
            }

            // try "wal" extension instead
            if (kdl::str_to_lower(skinPath.extension()) == "bmp") {
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

        void DkmParser::buildFrame(Assets::EntityModel& model, Assets::EntityModelSurface& surface, const size_t frameIndex, const DkmFrame& frame, const DkmMeshList& meshes) {
            size_t vertexCount = 0;
            Renderer::IndexRangeMap::Size size;
            for (const auto& md2Mesh : meshes) {
                vertexCount += md2Mesh.vertices.size();
                if (md2Mesh.type == DkmMesh::Fan) {
                    size.inc(Renderer::PrimType::TriangleFan);
                } else {
                    size.inc(Renderer::PrimType::TriangleStrip);
                }
            }

            vm::bbox3f::builder bounds;

            Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder(vertexCount, size);
            for (const auto& md2Mesh : meshes) {
                if (!md2Mesh.vertices.empty()) {
                    vertexCount += md2Mesh.vertices.size();
                    const auto vertices = getVertices(frame, md2Mesh.vertices);

                    bounds.add(std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent<0>());

                    if (md2Mesh.type == DkmMesh::Fan) {
                        builder.addTriangleFan(vertices);
                    } else {
                        builder.addTriangleStrip(vertices);
                    }
                }
            }

            auto& modelFrame = model.loadFrame(frameIndex, frame.name, bounds.bounds());
            surface.addIndexedMesh(modelFrame, builder.vertices(), builder.indices());
        }

        std::vector<Assets::EntityModelVertex> DkmParser::getVertices(const DkmFrame& frame, const DkmMeshVertexList& meshVertices) const {
            std::vector<Assets::EntityModelVertex> result;
            result.reserve(meshVertices.size());

            for (const DkmMeshVertex& md2MeshVertex : meshVertices) {
                const auto position = frame.vertex(md2MeshVertex.vertexIndex);
                const auto& texCoords = md2MeshVertex.texCoords;

                result.emplace_back(position, texCoords);
            }

            return result;
        }
    }
}
