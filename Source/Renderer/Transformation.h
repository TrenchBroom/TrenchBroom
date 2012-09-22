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

#ifndef TrenchBroom_Transformation_h
#define TrenchBroom_Transformation_h

#include <GL/glew.h>
#include "Renderer/Camera.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class Transformation {
        private:
            Mat4f::List m_matrixStack;
        public:
            Transformation(const Mat4f& matrix, bool load) {
                m_matrixStack.push_back(matrix);
                if (load)
                    loadMatrix(matrix);
            }
            
            ~Transformation() {
                assert(m_matrixStack.size() == 1);
            }
            
            inline Mat4f& pushMatrix() {
                m_matrixStack.push_back(m_matrixStack.back());
                return m_matrixStack.back();
            }
            
            inline void popMatrix() {
                assert(m_matrixStack.size() > 1);
                m_matrixStack.pop_back();
                loadMatrix(m_matrixStack.back());
            }
            
            inline void loadMatrix(const Mat4f& matrix) {
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(matrix.v);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
            }
        };
    }
}

#endif
