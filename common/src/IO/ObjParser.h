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

#ifndef TRENCHBROOM_OBJPARSER_H
#define TRENCHBROOM_OBJPARSER_H

#include "Assets/Asset_Forward.h"
#include "IO/EntityModelParser.h"
#include "IO/IO_Forward.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#include <vecmath/forward.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class ObjVertexRef {
        public:
            /**
             * Parses a vertex reference.
             *
             * @param text the text of this reference (such as "1/2/3")
             */
            ObjVertexRef(const std::string& text);
            // Position index (1-based. Should always be present.)
            size_t m_position;
            size_t m_texcoord;
        };

        class ObjFace {
        public:
            // The material of this face (as a skin index)
            size_t m_material;
            // The vertices of this face.
            std::vector<ObjVertexRef> m_vertices;
        };

        class ObjParser : public EntityModelParser {
        private:
            std::string m_name;
            std::string m_text;
            const FileSystem& m_fs;
        public:
            /**
             * Creates a new parser for Wavefront OBJ models.
             *
             * @param name the name of the model
             * @param begin the start of the text
             * @param end the end of the text
             */
            ObjParser(const std::string& name, const char* begin, const char* end, const FileSystem& fs);
        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
        };
    }
}


#endif //TRENCHBROOM_ASEPARSER_H
