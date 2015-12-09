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

#include "Transformation.h"

#include "Renderer/GL.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Transformation::Transformation(const Mat4x4f& projection, const Mat4x4f& view, const Mat4x4f& model) {
            pushTransformation(projection, view, model);
        }
        
        Transformation::~Transformation() {
            if (m_projectionStack.size() > 1)
                loadProjectionMatrix(m_projectionStack.front());
            if (m_viewStack.size() > 1 || m_modelStack.size() > 1)
                loadModelViewMatrix(m_viewStack.front() * m_modelStack.front());
        }
        
        Transformation Transformation::slice() const {
            return Transformation(m_projectionStack.back(), m_viewStack.back(), m_modelStack.back());
        }

        void Transformation::pushTransformation(const Mat4x4f& projection, const Mat4x4f& view, const Mat4x4f& model) {
            m_projectionStack.push_back(projection);
            m_viewStack.push_back(view);
            m_modelStack.push_back(model);
            
            loadProjectionMatrix(m_projectionStack.back());
            loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
        }
        
        void Transformation::popTransformation() {
            assert(m_projectionStack.size() > 1);
            assert(m_viewStack.size() > 1);
            assert(m_modelStack.size() > 1);
            
            m_projectionStack.pop_back();
            m_viewStack.pop_back();
            m_modelStack.pop_back();
            
            loadProjectionMatrix(m_projectionStack.back());
            loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
        }
        
        void Transformation::pushModelMatrix(const Mat4x4f& matrix) {
            m_modelStack.push_back(m_modelStack.back() * matrix);
            loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
        }
        
        void Transformation::replaceAndPushModelMatrix(const Mat4x4f& matrix) {
            m_modelStack.push_back(matrix);
            loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
        }
        
        void Transformation::popModelMatrix() {
            assert(m_modelStack.size() > 1);
            m_modelStack.pop_back();
            loadModelViewMatrix(m_viewStack.back() * m_modelStack.back());
        }

        void Transformation::loadProjectionMatrix(const Mat4x4f& matrix) {
            glAssert(glMatrixMode(GL_PROJECTION));
            glAssert(glLoadMatrixf(reinterpret_cast<const float*>(matrix.v)));
        }
        
        void Transformation::loadModelViewMatrix(const Mat4x4f& matrix) {
            glAssert(glMatrixMode(GL_MODELVIEW));
            glAssert(glLoadMatrixf(reinterpret_cast<const float*>(matrix.v)));
        }

        ReplaceTransformation::ReplaceTransformation(Transformation& transformation, const Mat4x4f& projectionMatrix, const Mat4x4f& viewMatrix, const Mat4x4f& modelMatrix) :
        m_transformation(transformation) {
            m_transformation.pushTransformation(projectionMatrix, viewMatrix, modelMatrix);
        }
        
        ReplaceTransformation::~ReplaceTransformation() {
            m_transformation.popTransformation();
        }
        
        MultiplyModelMatrix::MultiplyModelMatrix(Transformation& transformation, const Mat4x4f& modelMatrix) :
        m_transformation(transformation) {
            m_transformation.pushModelMatrix(modelMatrix);
        }
        
        MultiplyModelMatrix::~MultiplyModelMatrix() {
            m_transformation.popModelMatrix();
        }
        
        ReplaceModelMatrix::ReplaceModelMatrix(Transformation& transformation, const Mat4x4f& modelMatrix) :
        m_transformation(transformation) {
            m_transformation.replaceAndPushModelMatrix(modelMatrix);
        }
        
        ReplaceModelMatrix::~ReplaceModelMatrix() {
            m_transformation.popModelMatrix();
        }
    }
}
