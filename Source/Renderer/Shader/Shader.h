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

#ifndef __TrenchBroom__Shader__
#define __TrenchBroom__Shader__

#include <GL/glew.h>
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Texture;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class Shader {
        private:
            Utility::Console& m_console;
            String m_name;
            GLenum m_type;
            GLuint m_shaderId;
        public:
            static StringList loadSource(const String& path);
            
            Shader(const String& path, GLenum type, Utility::Console& console);
            ~Shader();
            
            void attachTo(GLuint programId);
            void detachFrom(GLuint programId);
        };
        
        class ShaderProgram {
        private:
            typedef std::map<String, GLint> UniformVariableMap;
            
            Utility::Console& m_console;
            String m_name;
            GLuint m_programId;
            UniformVariableMap m_uniformVariables;
            bool m_needsLinking;
            
            GLint uniformLocation(const String& name);
        public:
            ShaderProgram(const String& name, Utility::Console& console);
            ~ShaderProgram();
            

            inline GLuint programId() const {
                return m_programId;
            }
            
            void attachShader(Shader& shader);
            void detachShader(Shader& shader);

            bool activate();
            void deactivate();
            
            bool setUniformVariable(const String& name, bool value);
            bool setUniformVariable(const String& name, int value);
            bool setUniformVariable(const String& name, float value);
            bool setUniformVariable(const String& name, const Vec2f& value);
            bool setUniformVariable(const String& name, const Vec3f& value);
            bool setUniformVariable(const String& name, const Vec4f& value);
            bool setUniformVariable(const String& name, const Mat2f& value);
            bool setUniformVariable(const String& name, const Mat3f& value);
            bool setUniformVariable(const String& name, const Mat4f& value);
        };
    }
}

#endif /* defined(__TrenchBroom__Shader__) */
