/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__MapWriter__
#define __TrenchBroom__MapWriter__

#include "Model/ModelTypes.h"

#include <cstdio>
#include <ostream>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MapWriter {
        public:
            virtual ~MapWriter();
            
            void writeObjectsToStream(const Model::ObjectList& objects, std::ostream& stream);
            void writeFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream);
            void writeToStream(const Model::Map& map, std::ostream& stream);
            
            void writeToFileAtPath(Model::Map& map, const Path& path, const bool overwrite);
        private:
            size_t writeEntity(Model::Entity& entity, const Model::BrushList& brushes, const size_t lineNumber, FILE* stream);
            virtual size_t writeFace(Model::BrushFace& face, const size_t lineNumber, FILE* stream) = 0;
            virtual size_t writeBrush(Model::Brush& brush, const size_t lineNumber, FILE* stream);
            virtual size_t writeEntityHeader(Model::Entity& entity, FILE* stream);
            virtual size_t writeEntityProperty(const Model::EntityProperty& property, FILE* stream);
            virtual size_t writeEntityFooter(FILE* stream);
            virtual size_t writeExtraProperties(const Model::Object& object, FILE* stream);
            
            void writeEntity(const Model::Entity& entity, const Model::BrushList& brushes, std::ostream& stream);
            virtual void writeFace(const Model::BrushFace& face, std::ostream& stream) = 0;
            virtual void writeBrush(const Model::Brush& brush, std::ostream& stream);
            virtual void writeEntityHeader(const Model::Entity& entity, std::ostream& stream);
            virtual void writeEntityProperty(const Model::EntityProperty& property, std::ostream& stream);
            virtual void writeEntityFooter(std::ostream& stream);
            virtual void writeExtraProperties(const Model::Object& object, std::ostream& stream);
        };
    }
}

#endif /* defined(__TrenchBroom__MapWriter__) */
