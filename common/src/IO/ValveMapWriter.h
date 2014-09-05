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

#ifndef __TrenchBroom__ValveMapWriter__
#define __TrenchBroom__ValveMapWriter__

#include "IO/MapWriter.h"
#include "Model/ModelTypes.h"

#include <cstdio>
#include <ostream>

namespace TrenchBroom {
    namespace IO {
        class ValveMapWriter : public MapWriter {
        private:
            static const int FloatPrecision = 100;
            String FaceFormat;
        public:
            ValveMapWriter();
        private:
            virtual size_t writeFace(Model::BrushFace* face, size_t lineNumber, FILE* stream);
            virtual void writeFace(const Model::BrushFace* face, std::ostream& stream);
        };
    }
}

#endif /* defined(__TrenchBroom__ValveMapWriter__) */
