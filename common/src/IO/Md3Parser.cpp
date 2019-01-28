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

#include "Md3Parser.h"

#include "IO/CharArrayReader.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "Renderer/IndexRangeMapBuilder.h"

namespace TrenchBroom {
    namespace IO {
        namespace Md3Layout {
            static const int Ident = (('3'<<24) + ('P'<<16) + ('D'<<8) + 'I');
            static const int Version = 15;
            static const size_t ModelNameLength = 64;
            static const size_t FrameNameLength = 16;
            static const size_t FrameLength = 3 * 3 * sizeof(float) + sizeof(float) + FrameNameLength;
            // static const size_t TagNameLength = 64;
            // static const size_t TagLength = TagNameLength + 4 * 3 * sizeof(float);
            static const size_t SurfaceNameLength = 64;
            static const size_t TriangleLength = 3 * sizeof(int32_t);
            static const size_t ShaderNameLength = 64;
            static const size_t ShaderLength = ShaderNameLength + sizeof(int32_t);
            static const size_t TexCoordLength = 2 * sizeof(float);
            static const size_t VertexLength = 4 * sizeof(int16_t);
            static const float VertexScale = 1.0f / 64.0f;
        }

        Md3Parser::Md3Parser(const String& name, const char* begin, const char* end, const FileSystem& fs) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_fs(fs) {
            assert(m_begin < m_end);
            unused(m_end);
        }

        Assets::EntityModel* Md3Parser::doParseModel() {
            CharArrayReader reader(m_begin, m_end);

            const auto ident = reader.readInt<int32_t>();
            const auto version = reader.readInt<int32_t>();

            if (ident != Md3Layout::Ident) {
                throw AssetException() << "Unknown MD3 model ident: " << ident;
            }

            if (version != Md3Layout::Version) {
                throw AssetException() << "Unknown MD3 model version: " << version;
            }

            const auto name = reader.readString(Md3Layout::ModelNameLength);
            /* const auto flags = */ reader.readInt<int32_t>();

            const auto frameCount = reader.readSize<int32_t>();
            /* const auto tagCount = */ reader.readSize<int32_t>();
            const auto surfaceCount = reader.readSize<int32_t>();
            /* const auto skinCount = */ reader.readSize<int32_t>();

            const auto frameOffset = reader.readSize<int32_t>();
            /* const auto tagOffset = */ reader.readSize<int32_t>();
            const auto surfaceOffset = reader.readSize<int32_t>();

            auto model = std::make_unique<Assets::EntityModel>(m_name);

            parseFrames(reader.subReaderFromBegin(frameOffset, frameCount * Md3Layout::FrameLength), frameCount, *model);
            // parseTags(reader.subReaderFromBegin(tagOffset, tagCount * Md3Layout::TagLength), tagCount);
            parseSurfaces(reader.subReaderFromBegin(surfaceOffset), surfaceCount, *model);

            return model.release();
        }

        void Md3Parser::parseFrames(CharArrayReader reader, const size_t frameCount, Assets::EntityModel& model) {
            for (size_t i = 0; i < frameCount; ++i) {
                const auto minBounds = reader.readVec<float, 3>();
                const auto maxBounds = reader.readVec<float, 3>();
                /* const auto localOrigin = */ reader.readVec<float, 3>();
                /* const auto radius = */ reader.readFloat<float>();
                const auto frameName = reader.readString(Md3Layout::FrameNameLength);

                model.addFrame(frameName, vm::bbox3f(minBounds, maxBounds));
            }
        }

        /*
        void Md3Parser::parseTags(CharArrayReader reader, const size_t tagCount) {
            for (size_t i = 0; i < tagCount; ++i) {
                const auto tagName = reader.readString(Md3Layout::TagNameLength);
                const auto tagOrigin = reader.readVec<float, 3>();
                const auto tagAxis1 = reader.readVec<float, 3>();
                const auto tagAxis2 = reader.readVec<float, 3>();
                const auto tagAxis3 = reader.readVec<float, 3>();
            }
        }
         */

        void Md3Parser::parseSurfaces(CharArrayReader reader, const size_t surfaceCount, Assets::EntityModel& model) {
            for (size_t i = 0; i < surfaceCount; ++i) {
                const auto ident = reader.readInt<int32_t>();

                if (ident != Md3Layout::Ident) {
                    throw AssetException() << "Unknown MD3 model surface ident: " << ident;
                }

                const auto surfaceName = reader.readString(Md3Layout::SurfaceNameLength);
                /* const auto flags = */ reader.readInt<int32_t>();
                const auto frameCount = reader.readSize<int32_t>();
                const auto shaderCount = reader.readSize<int32_t>();
                const auto vertexCount = reader.readSize<int32_t>(); // the number of vertices per frame!
                const auto triangleCount = reader.readSize<int32_t>();
                const auto totalVertexCount = vertexCount * frameCount;

                const auto triangleOffset = reader.readSize<int32_t>();
                const auto shaderOffset = reader.readSize<int32_t>();
                const auto texCoordOffset = reader.readSize<int32_t>();
                const auto vertexOffset = reader.readSize<int32_t>(); // all vertices for all frames are stored there!
                /* const auto endOffset = */ reader.readSize<int32_t>();

                const auto vertices = parseVertices(
                    reader.subReaderFromBegin(vertexOffset, totalVertexCount * Md3Layout::VertexLength),
                    reader.subReaderFromBegin(texCoordOffset, totalVertexCount * Md3Layout::TexCoordLength),
                    totalVertexCount);

                const auto triangles = parseTriangles(reader.subReaderFromBegin(triangleOffset, triangleCount * Md3Layout::TriangleLength), triangleCount);
                const auto shaders = parseShaders(reader.subReaderFromBegin(shaderOffset, shaderCount * Md3Layout::ShaderLength), shaderCount);

                auto& surface = model.addSurface(surfaceName);
                loadSurfaceSkins(surface, shaders);
                buildSurfaceFrames(surface, triangles, vertices, frameCount, vertexCount);
            }
        }

        void Md3Parser::loadSurfaceSkins(Assets::EntityModel::Surface& surface, const std::vector<Path>& shaders) {
            TextureReader::PathSuffixNameStrategy nameStrategy(2, true);
            FreeImageTextureReader reader(nameStrategy);

            for (const auto& shader : shaders) {
                auto file = m_fs.openFile(shader);
                surface.addSkin(reader.readTexture(file));
            }
        }

        void Md3Parser::buildSurfaceFrames(Assets::EntityModel::Surface& surface, const std::vector<Md3Parser::Md3Triangle>& triangles, const std::vector<Assets::EntityModel::Vertex>& vertices, const size_t frameCount, const size_t vertexCountPerFrame) {
            using Vertex = Assets::EntityModel::Vertex;

            const auto rangeMap = Renderer::IndexRangeMap(GL_TRIANGLES, 0, 3 * triangles.size());
            for (size_t i = 0; i < frameCount; ++i) {
                const auto frameOffset = i * vertexCountPerFrame;

                std::vector<Vertex> frameVertices;
                frameVertices.reserve(3 * triangles.size());

                for (const auto& triangle : triangles) {
                    const auto& v1 = vertices[triangle.i1 + frameOffset];
                    const auto& v2 = vertices[triangle.i2 + frameOffset];
                    const auto& v3 = vertices[triangle.i3 + frameOffset];

                    frameVertices.push_back(v1);
                    frameVertices.push_back(v2);
                    frameVertices.push_back(v3);
                }

                surface.addIndexedFrame(frameVertices, rangeMap);
            }
        }

        std::vector<Md3Parser::Md3Triangle> Md3Parser::parseTriangles(CharArrayReader reader, const size_t triangleCount) {
            std::vector<Md3Triangle> result;
            result.reserve(triangleCount);
            for (size_t i = 0; i < triangleCount; ++i) {
                const auto i1 = reader.readSize<int32_t>();
                const auto i2 = reader.readSize<int32_t>();
                const auto i3 = reader.readSize<int32_t>();
                result.push_back(Md3Triangle {i1, i2, i3});
            }
            return result;
        }

        std::vector<Path> Md3Parser::parseShaders(CharArrayReader reader, const size_t shaderCount) {
            std::vector<Path> result;
            result.reserve(shaderCount);
            for (size_t i = 0; i < shaderCount; ++i) {
                const auto shaderName = reader.readString(Md3Layout::ShaderNameLength);
                /* const auto shaderIndex = */ reader.readSize<int32_t>();
                result.emplace_back(shaderName);
            }
            return result;
        }

        std::vector<Assets::EntityModel::Vertex> Md3Parser::parseVertices(CharArrayReader xyznReader, CharArrayReader stReader, size_t vertexCount) {
            using Vertex = Assets::EntityModel::Vertex;
            std::vector<Vertex> result;
            result.reserve(vertexCount);

            for (size_t i = 0; i < vertexCount; ++i) {
                const auto x = static_cast<float>(xyznReader.readInt<int16_t>()) * Md3Layout::VertexScale;
                const auto y = static_cast<float>(xyznReader.readInt<int16_t>()) * Md3Layout::VertexScale;
                const auto z = static_cast<float>(xyznReader.readInt<int16_t>()) * Md3Layout::VertexScale;
                /* const auto n = */ xyznReader.readInt<int16_t>();

                const auto s = stReader.readFloat<float>();
                const auto t = stReader.readFloat<float>();

                result.emplace_back(vm::vec3f(x, y, z), vm::vec2f(s, t));
            }

            return result;
        }
    }
}
