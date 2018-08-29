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

#ifndef TrenchBroom_Transformation
#define TrenchBroom_Transformation

#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Transformation {
        private:
            mat4x4f::List m_projectionStack;
            mat4x4f::List m_viewStack;
            mat4x4f::List m_modelStack;
        public:
            Transformation(const mat4x4f& projection, const mat4x4f& view, const mat4x4f& model = mat4x4f::identity);
            ~Transformation();
            
            Transformation slice() const;
            
            void pushTransformation(const mat4x4f& projection, const mat4x4f& view, const mat4x4f& model = mat4x4f::identity);
            void popTransformation();
            void pushModelMatrix(const mat4x4f& matrix);
            void replaceAndPushModelMatrix(const mat4x4f& matrix);
            void popModelMatrix();
        private:
            void loadProjectionMatrix(const mat4x4f& matrix);
            void loadModelViewMatrix(const mat4x4f& matrix);
        private:
            Transformation(const Transformation& other);
            Transformation& operator=(const Transformation& other);
        };

        class ReplaceTransformation {
        protected:
            Transformation& m_transformation;
        public:
            ReplaceTransformation(Transformation& transformation, const mat4x4f& projectionMatrix, const mat4x4f& viewMatrix, const mat4x4f& modelMatrix = mat4x4f::identity);
            ~ReplaceTransformation();
        private:
            ReplaceTransformation(const ReplaceTransformation& other);
            ReplaceTransformation& operator=(const ReplaceTransformation& other);
        };
        
        class MultiplyModelMatrix {
        protected:
            Transformation& m_transformation;
        public:
            MultiplyModelMatrix(Transformation& transformation, const mat4x4f& modelMatrix);
            ~MultiplyModelMatrix();
        private:
            MultiplyModelMatrix(const ReplaceTransformation& other);
            MultiplyModelMatrix& operator=(const ReplaceTransformation& other);
        };
        
        class ReplaceModelMatrix {
        protected:
            Transformation& m_transformation;
        public:
            ReplaceModelMatrix(Transformation& transformation, const mat4x4f& modelMatrix);
            ~ReplaceModelMatrix();
        private:
            ReplaceModelMatrix(const ReplaceTransformation& other);
            ReplaceModelMatrix& operator=(const ReplaceTransformation& other);
        };
    }
}

#endif /* defined(TrenchBroom_Transformation) */
