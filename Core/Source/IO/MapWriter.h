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

#include <string>
#include <ostream>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Face;
        class Map;
    }
    
    namespace IO {
        class MapWriter {
        protected:
            Model::Map& m_map;
            
            void writeFace(const Model::Face& face, std::ostream& stream);
            void writeBrush(const Model::Brush& brush, std::ostream& stream);
            void writeEntity(const Model::Entity& entity, std::ostream& stream);
        public:
            MapWriter(Model::Map& map) : m_map(map) {}
            
            void writeToStream(std::ostream& stream);
            void writeToFileAtPath(const std::string& path, bool overwrite);
        };
    }
}

#endif
