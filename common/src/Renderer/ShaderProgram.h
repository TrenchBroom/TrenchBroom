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

#ifndef TrenchBroom_ShaderProgram
#define TrenchBroom_ShaderProgram

#include "StringUtils.h"
#include "Vec.h"
#include "Mat.h"
#include "Renderer/GL.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class Shader;
        
        class ShaderProgram {
        private:
            typedef std::map<String, GLint> UniformVariableCache;
            
            String m_name;
            GLuint m_programId;
            bool m_needsLinking;
            mutable UniformVariableCache m_variableCache;
        public:
            ShaderProgram(const String& name);
            ~ShaderProgram();
            
            void attach(Shader& shader);
            void detach(Shader& shader);
            
            void activate();
            void deactivate();

            void set(const String& name, const bool value);
            void set(const String& name, const int value);
            void set(const String& name, const size_t value);
            void set(const String& name, const float value);
            void set(const String& name, const double value);
            void set(const String& name, const Vec2f& value);
            void set(const String& name, const Vec3f& value);
            void set(const String& name, const Vec4f& value);
            void set(const String& name, const Mat2x2f& value);
            void set(const String& name, const Mat3x3f& value);
            void set(const String& name, const Mat4x4f& value);
        private:
            void link();
            GLint findUniformLocation(const String& name) const;
            bool checkActive() const;
        };
    }
}

#endif /* defined(TrenchBroom_ShaderProgram) */
