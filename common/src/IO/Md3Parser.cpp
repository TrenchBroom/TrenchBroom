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

#include "Macros.h"
#include "Exceptions.h"

#include "IO/CharArrayReader.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <cassert>
#include <cstdint>

namespace TrenchBroom {
    namespace IO {
        namespace Md3Layout {
            static const int Ident = (('3'<<24) + ('P'<<16) + ('D'<<8) + 'I');
            static const int Version = 15;
            static const size_t ModelNameLength = 64;
            static const size_t FrameNameLength = 16;
            static const size_t FrameLength = 3 * 3 * sizeof(float) + sizeof(float) + FrameNameLength;
            static const size_t TagNameLength = 64;
            static const size_t TagLength = TagNameLength + 4 * 3 * sizeof(float);
            static const size_t SurfaceNameLength = 64;
            static const size_t TriangleLength = 3 * sizeof(int32_t);
            static const size_t ShaderNameLength = 64;
            static const size_t ShaderLength = ShaderNameLength + sizeof(int32_t);
            static const size_t TexCoordLength = 2 * sizeof(float);
            static const size_t VertexLength = 4 * sizeof(int16_t);
        }

        Md3Parser::Md3Parser(const String& name, const char* begin, const char* end) :
        m_name(name),
        m_begin(begin),
        m_end(end) {
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
            const auto flags = reader.readInt<int32_t>();

            const auto frameCount = reader.readSize<int32_t>();
            const auto tagCount = reader.readSize<int32_t>();
            const auto surfaceCount = reader.readSize<int32_t>();
            const auto skinCount = reader.readSize<int32_t>();

            const auto frameOffset = reader.readSize<int32_t>();
            const auto tagOffset = reader.readSize<int32_t>();
            const auto surfaceOffset = reader.readSize<int32_t>();

            parseFrames(reader.subReaderFromBegin(frameOffset, frameCount * Md3Layout::FrameLength), frameCount);
            parseTags(reader.subReaderFromBegin(tagOffset, tagCount * Md3Layout::TagLength), tagCount);
            parseSurfaces(reader.subReaderFromBegin(surfaceOffset), surfaceCount);
        }

        void Md3Parser::parseFrames(CharArrayReader reader, const size_t frameCount) {
            for (size_t i = 0; i < frameCount; ++i) {
                const auto minBounds = reader.readVec<float, 3>();
                const auto maxBounds = reader.readVec<float, 3>();
                const auto localOrigin = reader.readVec<float, 3>();
                const auto radius = reader.readFloat<float>();
                const auto frameName = reader.readString(Md3Layout::FrameNameLength);
            }
        }

        void Md3Parser::parseTags(CharArrayReader reader, const size_t tagCount) {
            for (size_t i = 0; i < tagCount; ++i) {
                const auto tagName = reader.readString(Md3Layout::TagNameLength);
                const auto tagOrigin = reader.readVec<float, 3>();
                const auto tagAxis1 = reader.readVec<float, 3>();
                const auto tagAxis2 = reader.readVec<float, 3>();
                const auto tagAxis3 = reader.readVec<float, 3>();
            }
        }

        void Md3Parser::parseSurfaces(CharArrayReader reader, const size_t surfaceCount) {
            for (size_t i = 0; i < surfaceCount; ++i) {
                const auto ident = reader.readInt<int32_t>();

                if (ident != Md3Layout::Ident) {
                    throw AssetException() << "Unknown MD3 model surface ident: " << ident;
                }

                const auto surfaceName = reader.readString(Md3Layout::SurfaceNameLength);
                const auto flags = reader.readInt<int32_t>();
                const auto frameCount = reader.readSize<int32_t>();
                const auto shaderCount = reader.readSize<int32_t>();
                const auto vertexCount = reader.readSize<int32_t>();
                const auto triangleCount = reader.readSize<int32_t>();

                const auto triangleOffset = reader.readSize<int32_t>();
                const auto shaderOffset = reader.readSize<int32_t>();
                const auto texCoordOffset = reader.readSize<int32_t>();
                const auto vertexOffset = reader.readSize<int32_t>();
                const auto endOffset = reader.readSize<int32_t>();

                parseTriangles(reader.subReaderFromBegin(triangleOffset, triangleCount * Md3Layout::TriangleLength), triangleCount);
                parseShaders(reader.subReaderFromBegin(shaderOffset, shaderCount * Md3Layout::ShaderLength), shaderCount);
                parseTexCoords(reader.subReaderFromBegin(texCoordOffset, vertexCount * Md3Layout::TexCoordLength), vertexCount);
                parseVertices(reader.subReaderFromBegin(vertexOffset, vertexCount * Md3Layout::VertexLength), vertexCount);
            }
        }
    }
}
