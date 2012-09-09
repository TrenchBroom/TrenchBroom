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

#ifndef TrenchBroom_PushMatrix_h
#define TrenchBroom_PushMatrix_h

#include "Renderer/Transformation.h"
#include "Utility/GLee.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class PushMatrix {
        protected:
            Transformation& m_transformation;
            Mat4f& m_matrix;
        public:
            PushMatrix(Transformation& transformation) :
            m_transformation(transformation),
            m_matrix(m_transformation.pushMatrix()) {}
            
            ~PushMatrix() {
                m_transformation.popMatrix();
            }
            
            inline const Mat4f& matrix() const {
                return m_matrix;
            }
            
            inline void load(const Mat4f& matrix) {
                m_transformation.loadMatrix(matrix);
            }
        };
    }
}

#endif
