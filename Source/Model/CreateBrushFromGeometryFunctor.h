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

#ifndef TrenchBroom_CreateBrushFromGeometryFunctor_h
#define TrenchBroom_CreateBrushFromGeometryFunctor_h

#include "IO/ByteBuffer.h"
#include "Model/Brush.h"
#include "Model/Face.h"
#include "Utility/VecMath.h"

#include <algorithm>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class CreateBrushFromGeometryFunctor : public CreateBrushFunctor {
        private:
            const BBox m_worldBounds;
            IO::ByteBuffer& m_buffer;
        public:
            CreateBrushFromGeometryFunctor(const BBox& worldBounds, IO::ByteBuffer& buffer) :
            m_worldBounds(worldBounds),
            m_buffer(buffer) {}
            
            Brush* operator()(const Model::FaceList& faces) {
                Brush* brush = new Brush(m_worldBounds);

                brush->setFaces(faces);
                brush->deserializeGeometry(m_buffer);

                return brush;
            }
        };
    }
}


#endif
