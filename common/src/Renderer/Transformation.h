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

#ifndef TrenchBroom_Transformation
#define TrenchBroom_Transformation

#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Transformation {
        private:
            Mat4x4f::List m_projectionStack;
            Mat4x4f::List m_viewStack;
            Mat4x4f::List m_modelStack;
        public:
            Transformation(const Mat4x4f& projection, const Mat4x4f& view, const Mat4x4f& model = Mat4x4f::Identity);
            ~Transformation();
            
            Transformation slice() const;
            
            void pushTransformation(const Mat4x4f& projection, const Mat4x4f& view, const Mat4x4f& model = Mat4x4f::Identity);
            void popTransformation();
            void pushModelMatrix(const Mat4x4f& matrix);
            void replaceAndPushModelMatrix(const Mat4x4f& matrix);
            void popModelMatrix();
        private:
            void loadProjectionMatrix(const Mat4x4f& matrix);
            void loadModelViewMatrix(const Mat4x4f& matrix);
        };

        class ReplaceTransformation {
        protected:
            Transformation& m_transformation;
        public:
            ReplaceTransformation(Transformation& transformation, const Mat4x4f& projectionMatrix, const Mat4x4f& viewMatrix, const Mat4x4f& modelMatrix = Mat4x4f::Identity);
            ~ReplaceTransformation();
        };
        
        class MultiplyModelMatrix {
        protected:
            Transformation& m_transformation;
        public:
            MultiplyModelMatrix(Transformation& transformation, const Mat4x4f& modelMatrix);
            ~MultiplyModelMatrix();
        };
        
        class ReplaceModelMatrix {
        protected:
            Transformation& m_transformation;
        public:
            ReplaceModelMatrix(Transformation& transformation, const Mat4x4f& modelMatrix);
            ~ReplaceModelMatrix();
        };
    }
}

#endif /* defined(TrenchBroom_Transformation) */
