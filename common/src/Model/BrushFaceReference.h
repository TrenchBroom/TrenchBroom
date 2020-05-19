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

#ifndef BrushFaceReference_h
#define BrushFaceReference_h

#include "FloatType.h"

#include <vecmath/plane.h>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;

        class BrushFaceReference {
        private:
            vm::plane3 m_facePlane;
            Model::Brush* m_brush;
        public:
            BrushFaceReference(Model::BrushFace* face);

            Model::BrushFace* resolve() const;
        };
    }
}

#endif /* BrushFaceReference_h */
