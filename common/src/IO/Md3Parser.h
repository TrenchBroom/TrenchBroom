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

#include "StringUtils.h"
#include "IO/EntityModelParser.h"

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
    }

    namespace IO {
        class CharArrayReader;

        class Md3Parser : public EntityModelParser {
        private:
            String m_name;
            const char* m_begin;
            const char* m_end;
        public:
            Md3Parser(const String& name, const char* begin, const char* end);
        private:
            Assets::EntityModel* doParseModel() override;

            void parseFrames(CharArrayReader reader, size_t frameCount);
            void parseTags(CharArrayReader reader, size_t tagCount);
            void parseSurfaces(CharArrayReader reader, size_t surfaceCount);
            void parseTriangles(CharArrayReader reader, size_t triangleCount);
            void parseShaders(CharArrayReader reader, size_t shaderCount);
            void parseTexCoords(CharArrayReader reader, size_t texCoordCount);
            void parseVertices(CharArrayReader reader, size_t vertexCount);
        };
    }
}


#endif //TRENCHBROOM_MD3PARSER_H
