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

#include "BrushFaceReference.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        BrushFaceReference::BrushFaceReference(Model::BrushFace* face) :
        m_facePlane(face->boundary()),
        m_brush(face->brush()) {
            ensure(m_brush != nullptr, "face without a brush");
        }

        Model::BrushFace* BrushFaceReference::resolve() const {
            Model::BrushFace* face = m_brush->findFace(m_facePlane);
            ensure(face != nullptr, "couldn't find face");
            return face;
        }
    }
}
