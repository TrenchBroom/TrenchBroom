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

#ifndef TrenchBroom_ApplyMatrix_h
#define TrenchBroom_ApplyMatrix_h

#include <GL/glew.h>
#include "Renderer/Transformation.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class ApplyMatrix {
        protected:
            Transformation& m_transformation;
        public:
            ApplyMatrix(Transformation& transformation, const Mat4f& matrix, bool replace = false) :
            m_transformation(transformation) {
                const Mat4f& currentMatrix = m_transformation.pushMatrix();
                m_transformation.loadMatrix(replace ? matrix : currentMatrix *= matrix);
            }
            
            ~ApplyMatrix() {
                m_transformation.popMatrix();
            }
        };
    }
}

#endif
