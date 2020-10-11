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

#ifndef TrenchBroom_MdlParser
#define TrenchBroom_MdlParser

#include "Assets/EntityModel_Forward.h"
#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Palette;
    }

    namespace IO {
        class BufferedReader;
        class Reader;

        class MdlParser : public EntityModelParser {
        private:
            static const vm::vec3f Normals[162];

            struct MdlSkinVertex {
                bool onseam;
                int s;
                int t;
            };

            struct MdlSkinTriangle {
                bool front;
                size_t vertices[3];
            };

            using MdlSkinVertexList = std::vector<MdlSkinVertex>;
            using MdlSkinTriangleList = std::vector<MdlSkinTriangle>;
            using PackedFrameVertex = vm::vec<unsigned char, 4>;
            using PackedFrameVertexList = std::vector<PackedFrameVertex>;

            std::string m_name;
            const char* m_begin;
            const char* m_end;
            const Assets::Palette& m_palette;
        public:
            MdlParser(const std::string& name, const char* begin, const char* end, const Assets::Palette& palette);
        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
            void doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;

            void parseSkins(BufferedReader& reader, Assets::EntityModelSurface& surface, size_t count, size_t width, size_t height, int flags);
            void skipSkins(Reader& reader, size_t count, size_t width, size_t height, int flags);

            MdlSkinVertexList parseVertices(Reader& reader, size_t count);
            MdlSkinTriangleList parseTriangles(Reader& reader, size_t count);

            void skipFrames(Reader& reader, size_t count, size_t vertexCount);
            void parseFrame(Reader& reader, Assets::EntityModel& model, size_t frameIndex, Assets::EntityModelSurface& surface, const MdlSkinTriangleList& triangles, const MdlSkinVertexList& vertices, size_t skinWidth, size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale);
            void doParseFrame(Reader reader, Assets::EntityModel& model, size_t frameIndex, Assets::EntityModelSurface& surface, const MdlSkinTriangleList& triangles, const MdlSkinVertexList& vertices, size_t skinWidth, size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale);
            vm::vec3f unpackFrameVertex(const PackedFrameVertex& vertex, const vm::vec3f& origin, const vm::vec3f& scale) const;
        };
    }
}

#endif /* defined(TrenchBroom_MdlParser) */
