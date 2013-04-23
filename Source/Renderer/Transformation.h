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

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        class Transformation {
        private:
            Mat4f::List m_projectionStack;
            Mat4f::List m_viewStack;
            Mat4f::List m_modelStack;
            
            inline void loadProjectionMatrix(const Mat4f& matrix) {
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(matrix.v);
            }

            inline void loadModelViewMatrix(const Mat4f& matrix) {
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(matrix.v);
            }
        public:
            Transformation(const Mat4f& projection, const Mat4f& view, const Mat4f& model = Mat4f::Identity) {
                pushTransformation(projection, view, model);
            }
            
            ~Transformation() {
                assert(m_projectionStack.size() == 1);
                assert(m_viewStack.size() == 1);
                assert(m_modelStack.size() == 1);
            }
            
            inline void pushTransformation(const Mat4f& projection, const Mat4f& view, const Mat4f& model = Mat4f::Identity) {
                m_projectionStack.push_back(projection);
                m_viewStack.push_back(view);
                m_modelStack.push_back(model);
                loadProjectionMatrix(m_projectionStack.back());
                loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
            }
            
            inline void popTransformation() {
                assert(m_projectionStack.size() > 1);
                assert(m_viewStack.size() > 1);
                assert(m_modelStack.size() > 1);
                m_projectionStack.pop_back();
                m_viewStack.pop_back();
                m_modelStack.pop_back();
                loadProjectionMatrix(m_projectionStack.back());
                loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
            }
            
            inline void pushModelMatrix(const Mat4f& matrix, bool replace) {
                m_modelStack.push_back(replace ? matrix : m_modelStack.back() * matrix);
                loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
            }
            
            inline void popModelMatrix() {
                assert(m_modelStack.size() > 1);
                m_modelStack.pop_back();
                loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
            }
        };
    }
}

#endif
