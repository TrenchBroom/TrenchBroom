/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MapWriter {
        public:
            virtual ~MapWriter();
            
            virtual void writeNodesToStream(const Model::NodeList& nodes, std::ostream& stream);
            virtual void writeFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream);
            
            virtual void writeToFileAtPath(Model::World* map, const Path& path, const bool overwrite);
        private:
            virtual size_t writeDefaultLayer(const Model::Attributable* worldspawn, const Model::Layer* defaultLayer, size_t lineNumber, FILE* stream);
        };
    }
}

#endif /* defined(__TrenchBroom__MapWriter__) */
