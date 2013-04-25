/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_MapWriter_h
#define TrenchBroom_MapWriter_h

#include "Model/EntityTypes.h"
#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"
#include "Utility/String.h"

#include <cstdio>
#include <ostream>

#if defined _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Face;
        class Map;
    }
    
    namespace IO {
        class MapWriter {
        private:
            static const int FloatPrecision = 100;
            String FaceFormat;
        protected:
            size_t writeFace(Model::Face& face, const size_t lineNumber, FILE* stream);
            size_t writeBrush(Model::Brush& brush, const size_t lineNumber, FILE* stream);
            size_t writeEntityHeader(Model::Entity& entity, FILE* stream);
            size_t writeEntityFooter(FILE* stream);
            size_t writeEntity(Model::Entity& entity, const size_t lineNumber, FILE* stream);
            
            void writeFace(const Model::Face& face, std::ostream& stream);
            void writeBrush(const Model::Brush& brush, std::ostream& stream);
            void writeEntityHeader(const Model::Entity& entity, std::ostream& stream);
            void writeEntityFooter(std::ostream& stream);
            void writeEntity(const Model::Entity& entity, std::ostream& stream);
        public:
            MapWriter();
            void writeObjectsToStream(const Model::EntityList& pointEntities, const Model::BrushList& brushes, std::ostream& stream);
            void writeFacesToStream(const Model::FaceList& faces, std::ostream& stream);
            void writeToStream(const Model::Map& map, std::ostream& stream);
            void writeToFileAtPath(Model::Map& map, const String& path, bool overwrite);
        };
    }
}

#endif
