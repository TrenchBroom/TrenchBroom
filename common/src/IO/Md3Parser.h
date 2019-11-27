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

#ifndef TRENCHBROOM_MD3PARSER_H
#define TRENCHBROOM_MD3PARSER_H

#include "StringType.h"
#include "Assets/EntityModel.h"
#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
    }

    namespace IO {
        class Reader;
        class FileSystem;
        class Path;

        class Md3Parser : public EntityModelParser {
        private:
            String m_name;
            const char* m_begin;
            const char* m_end;
            const FileSystem& m_fs;
        private:
            struct Md3Triangle {
                size_t i1, i2, i3;
            };
        public:
            Md3Parser(const String& name, const char* begin, const char* end, const FileSystem& fs);
        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
            void doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;

            void parseSurfaces(Reader surfaceReader, size_t surfaceCount, Assets::EntityModel& model, Logger& logger);
            Assets::EntityModel::LoadedFrame& parseFrame(Reader frameReader, size_t frameIndex, Assets::EntityModel& model);
            void parseFrameSurfaces(Reader surfaceReader, Assets::EntityModel::LoadedFrame& frame, Assets::EntityModel& model);

            std::vector<Md3Triangle> parseTriangles(Reader reader, size_t triangleCount);
            std::vector<Path> parseShaders(Reader reader, size_t shaderCount);
            std::vector<vm::vec3f> parseVertexPositions(Reader reader, size_t vertexCount);
            std::vector<vm::vec2f> parseTexCoords(Reader reader, size_t vertexCount);
            std::vector<Assets::EntityModel::Vertex> buildVertices(const std::vector<vm::vec3f>& positions, const std::vector<vm::vec2f>& texCoords);

            void loadSurfaceSkins(Assets::EntityModel::Surface& surface, const std::vector<Path>& shaders, Logger& logger);
            void buildFrameSurface(Assets::EntityModel::LoadedFrame& frame, Assets::EntityModel::Surface& surface, const std::vector<Md3Parser::Md3Triangle>& triangles, const std::vector<Assets::EntityModel::Vertex>& vertices);
        };
    }
}


#endif //TRENCHBROOM_MD3PARSER_H
