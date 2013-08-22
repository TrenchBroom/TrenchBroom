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

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class ApplyTransformation {
        protected:
            Transformation& m_transformation;
        public:
            ApplyTransformation(Transformation& transformation, const Mat4f& projectionMatrix, const Mat4f& viewMatrix, const Mat4f& modelMatrix = Mat4f::Identity) :
            m_transformation(transformation) {
                m_transformation.pushTransformation(projectionMatrix, viewMatrix, modelMatrix);
            }

            ~ApplyTransformation() {
                m_transformation.popTransformation();
            }
        };

        class ApplyModelMatrix {
        protected:
            Transformation& m_transformation;
        public:
            ApplyModelMatrix(Transformation& transformation, const Mat4f& modelMatrix, bool replace = false) :
            m_transformation(transformation) {
                m_transformation.pushModelMatrix(modelMatrix, replace);
            }
            
            ~ApplyModelMatrix() {
                m_transformation.popModelMatrix();
            }
        };
    }
}

#endif
